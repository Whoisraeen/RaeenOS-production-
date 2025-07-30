#!/usr/bin/env python3
"""
RaeenOS Infrastructure Monitoring System
Monitors CI/CD infrastructure health, performance, and alerts on issues
"""

import json
import time
import requests
import subprocess
import logging
import os
import sys
from datetime import datetime, timedelta
from dataclasses import dataclass, asdict
from typing import Dict, List, Optional, Any
import yaml

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('/var/log/raeenos-infrastructure-monitor.log'),
        logging.StreamHandler(sys.stdout)
    ]
)
logger = logging.getLogger(__name__)

@dataclass
class MetricValue:
    """Represents a metric value with timestamp"""
    value: float
    timestamp: datetime
    unit: str = ""
    metadata: Dict[str, Any] = None

@dataclass
class AlertConfig:
    """Alert configuration for a metric"""
    metric_name: str
    threshold_value: float
    comparison: str  # "gt", "lt", "eq", "ne"
    severity: str    # "critical", "warning", "info"
    cooldown_minutes: int = 15
    last_alert_time: Optional[datetime] = None

@dataclass
class InfrastructureHealth:
    """Overall infrastructure health status"""
    timestamp: datetime
    github_actions_status: str
    build_queue_length: int
    test_success_rate: float
    deployment_status: str
    artifact_storage_usage: float
    container_registry_status: str
    alerts_active: int
    overall_status: str

class GitHubActionsMonitor:
    """Monitor GitHub Actions workflow status and performance"""
    
    def __init__(self, token: str, repo: str):
        self.token = token
        self.repo = repo
        self.headers = {
            'Authorization': f'token {token}',
            'Accept': 'application/vnd.github.v3+json'
        }
        self.base_url = f'https://api.github.com/repos/{repo}'
    
    def get_workflow_runs(self, hours: int = 24) -> List[Dict]:
        """Get recent workflow runs"""
        since = (datetime.utcnow() - timedelta(hours=hours)).isoformat() + 'Z'
        
        url = f'{self.base_url}/actions/runs'
        params = {
            'per_page': 100,
            'created': f'>={since}'
        }
        
        try:
            response = requests.get(url, headers=self.headers, params=params)
            response.raise_for_status()
            return response.json().get('workflow_runs', [])
        except Exception as e:
            logger.error(f"Failed to get workflow runs: {e}")
            return []
    
    def get_queue_length(self) -> int:
        """Get current build queue length"""
        runs = self.get_workflow_runs(hours=1)
        queued_runs = [run for run in runs if run['status'] in ['queued', 'in_progress']]
        return len(queued_runs)
    
    def get_success_rate(self, hours: int = 24) -> float:
        """Get test success rate over specified period"""
        runs = self.get_workflow_runs(hours=hours)
        
        if not runs:
            return 0.0
        
        completed_runs = [run for run in runs if run['status'] == 'completed']
        if not completed_runs:
            return 0.0
        
        successful_runs = [run for run in completed_runs if run['conclusion'] == 'success']
        return (len(successful_runs) / len(completed_runs)) * 100.0
    
    def get_average_build_time(self, hours: int = 24) -> float:
        """Get average build time in minutes"""
        runs = self.get_workflow_runs(hours=hours)
        completed_runs = [run for run in runs if run['status'] == 'completed']
        
        if not completed_runs:
            return 0.0
        
        total_duration = 0
        for run in completed_runs:
            created = datetime.fromisoformat(run['created_at'].replace('Z', '+00:00'))
            updated = datetime.fromisoformat(run['updated_at'].replace('Z', '+00:00'))
            duration = (updated - created).total_seconds() / 60  # Convert to minutes
            total_duration += duration
        
        return total_duration / len(completed_runs)
    
    def get_runner_capacity(self) -> Dict[str, Any]:
        """Get GitHub Actions runner capacity information"""
        # This would typically query GitHub Enterprise API for runner info
        # For GitHub.com, we'll estimate based on queue length and recent performance
        queue_length = self.get_queue_length()
        avg_build_time = self.get_average_build_time(hours=1)
        
        return {
            'queue_length': queue_length,
            'estimated_wait_time_minutes': queue_length * (avg_build_time / 5),  # Assuming 5 parallel runners
            'capacity_utilization': min(100, (queue_length / 20) * 100),  # Assume 20 runner capacity
            'status': 'healthy' if queue_length < 10 else 'degraded' if queue_length < 20 else 'critical'
        }

class ArtifactStorageMonitor:
    """Monitor artifact storage usage and performance"""
    
    def __init__(self, github_monitor: GitHubActionsMonitor):
        self.github_monitor = github_monitor
    
    def get_storage_usage(self) -> Dict[str, Any]:
        """Get artifact storage usage statistics"""
        try:
            # Get repository storage info
            url = f'{self.github_monitor.base_url}'
            response = requests.get(url, headers=self.github_monitor.headers)
            response.raise_for_status()
            repo_data = response.json()
            
            # Calculate artifact storage (rough estimate)
            total_size_kb = repo_data.get('size', 0)
            
            # Get artifacts from recent runs
            artifacts_url = f'{self.github_monitor.base_url}/actions/artifacts'
            artifacts_response = requests.get(artifacts_url, headers=self.github_monitor.headers)
            artifacts_response.raise_for_status()
            artifacts_data = artifacts_response.json()
            
            total_artifacts = artifacts_data.get('total_count', 0)
            artifacts = artifacts_data.get('artifacts', [])
            
            total_artifact_size = sum(artifact.get('size_in_bytes', 0) for artifact in artifacts)
            
            return {
                'total_size_mb': total_artifact_size / (1024 * 1024),
                'artifact_count': total_artifacts,
                'usage_percentage': min(100, (total_artifact_size / (10 * 1024 * 1024 * 1024)) * 100),  # Assume 10GB limit
                'status': 'healthy' if total_artifact_size < 8 * 1024 * 1024 * 1024 else 'warning'
            }
        except Exception as e:
            logger.error(f"Failed to get storage usage: {e}")
            return {
                'total_size_mb': 0,
                'artifact_count': 0,
                'usage_percentage': 0,
                'status': 'unknown'
            }

class ContainerRegistryMonitor:
    """Monitor container registry status and usage"""
    
    def __init__(self, registry_url: str = "ghcr.io"):
        self.registry_url = registry_url
    
    def check_registry_health(self) -> Dict[str, Any]:
        """Check container registry health"""
        try:
            # Simple health check by trying to connect
            response = requests.get(f'https://{self.registry_url}', timeout=10)
            status = 'healthy' if response.status_code < 500 else 'degraded'
        except Exception as e:
            logger.error(f"Registry health check failed: {e}")
            status = 'unhealthy'
        
        return {
            'status': status,
            'registry_url': self.registry_url,
            'last_checked': datetime.utcnow().isoformat()
        }

class AlertManager:
    """Manage alerts and notifications"""
    
    def __init__(self, config_file: str = 'alert-config.yaml'):
        self.alerts: List[AlertConfig] = []
        self.active_alerts: Dict[str, AlertConfig] = {}
        self.load_config(config_file)
    
    def load_config(self, config_file: str):
        """Load alert configuration from file"""
        try:
            if os.path.exists(config_file):
                with open(config_file, 'r') as f:
                    config = yaml.safe_load(f)
                    
                for alert_config in config.get('alerts', []):
                    alert = AlertConfig(**alert_config)
                    self.alerts.append(alert)
                    
                logger.info(f"Loaded {len(self.alerts)} alert configurations")
            else:
                # Create default configuration
                self.create_default_config(config_file)
        except Exception as e:
            logger.error(f"Failed to load alert config: {e}")
    
    def create_default_config(self, config_file: str):
        """Create default alert configuration"""
        default_alerts = [
            {
                'metric_name': 'build_queue_length',
                'threshold_value': 10,
                'comparison': 'gt',
                'severity': 'warning',
                'cooldown_minutes': 15
            },
            {
                'metric_name': 'test_success_rate',
                'threshold_value': 80.0,
                'comparison': 'lt',
                'severity': 'critical',
                'cooldown_minutes': 30
            },
            {
                'metric_name': 'artifact_storage_usage',
                'threshold_value': 80.0,
                'comparison': 'gt',
                'severity': 'warning',
                'cooldown_minutes': 60
            }
        ]
        
        config = {'alerts': default_alerts}
        
        try:
            with open(config_file, 'w') as f:
                yaml.dump(config, f, default_flow_style=False)
            logger.info(f"Created default alert configuration: {config_file}")
        except Exception as e:
            logger.error(f"Failed to create default config: {e}")
    
    def check_metric(self, metric_name: str, value: float):
        """Check metric against alert thresholds"""
        for alert in self.alerts:
            if alert.metric_name != metric_name:
                continue
            
            # Check if alert is in cooldown
            if (alert.last_alert_time and 
                datetime.utcnow() - alert.last_alert_time < timedelta(minutes=alert.cooldown_minutes)):
                continue
            
            # Check threshold
            triggered = False
            if alert.comparison == 'gt' and value > alert.threshold_value:
                triggered = True
            elif alert.comparison == 'lt' and value < alert.threshold_value:
                triggered = True
            elif alert.comparison == 'eq' and value == alert.threshold_value:
                triggered = True
            elif alert.comparison == 'ne' and value != alert.threshold_value:
                triggered = True
            
            if triggered:
                self.trigger_alert(alert, value)
    
    def trigger_alert(self, alert: AlertConfig, current_value: float):
        """Trigger an alert"""
        alert.last_alert_time = datetime.utcnow()
        self.active_alerts[alert.metric_name] = alert
        
        message = f"ALERT [{alert.severity.upper()}]: {alert.metric_name} = {current_value} (threshold: {alert.comparison} {alert.threshold_value})"
        logger.warning(message)
        
        # Send notifications
        self.send_notifications(alert, current_value, message)
    
    def send_notifications(self, alert: AlertConfig, current_value: float, message: str):
        """Send alert notifications"""
        # Slack notification
        self.send_slack_notification(message, alert.severity)
        
        # Email notification (if configured)
        # self.send_email_notification(message, alert.severity)
        
        # Write to alert log
        self.log_alert(alert, current_value, message)
    
    def send_slack_notification(self, message: str, severity: str):
        """Send Slack notification"""
        webhook_url = os.getenv('SLACK_WEBHOOK_URL')
        if not webhook_url:
            logger.debug("Slack webhook URL not configured")
            return
        
        color = {
            'critical': '#FF0000',
            'warning': '#FFA500',
            'info': '#00FF00'
        }.get(severity, '#808080')
        
        payload = {
            'attachments': [{
                'color': color,
                'title': 'RaeenOS Infrastructure Alert',
                'text': message,
                'ts': int(time.time())
            }]
        }
        
        try:
            response = requests.post(webhook_url, json=payload, timeout=10)
            response.raise_for_status()
            logger.info("Slack notification sent successfully")
        except Exception as e:
            logger.error(f"Failed to send Slack notification: {e}")
    
    def log_alert(self, alert: AlertConfig, current_value: float, message: str):
        """Log alert to file"""
        alert_entry = {
            'timestamp': datetime.utcnow().isoformat(),
            'metric_name': alert.metric_name,
            'current_value': current_value,
            'threshold_value': alert.threshold_value,
            'comparison': alert.comparison,
            'severity': alert.severity,
            'message': message
        }
        
        try:
            with open('/var/log/raeenos-alerts.log', 'a') as f:
                f.write(json.dumps(alert_entry) + '\n')
        except Exception as e:
            logger.error(f"Failed to log alert: {e}")
    
    def clear_alert(self, metric_name: str):
        """Clear an active alert"""
        if metric_name in self.active_alerts:
            del self.active_alerts[metric_name]
            logger.info(f"Cleared alert for metric: {metric_name}")

class InfrastructureMonitor:
    """Main infrastructure monitoring coordinator"""
    
    def __init__(self, config_file: str = 'monitor-config.yaml'):
        self.config = self.load_config(config_file)
        
        # Initialize monitors
        self.github_monitor = GitHubActionsMonitor(
            self.config['github']['token'],
            self.config['github']['repository']
        )
        self.storage_monitor = ArtifactStorageMonitor(self.github_monitor)
        self.registry_monitor = ContainerRegistryMonitor()
        self.alert_manager = AlertManager()
        
        # Metrics storage
        self.metrics: Dict[str, List[MetricValue]] = {}
        
        logger.info("Infrastructure monitor initialized")
    
    def load_config(self, config_file: str) -> Dict[str, Any]:
        """Load monitoring configuration"""
        default_config = {
            'github': {
                'token': os.getenv('GITHUB_TOKEN', ''),
                'repository': os.getenv('GITHUB_REPOSITORY', 'raeenos/raeenos')
            },
            'monitoring': {
                'interval_seconds': 900,  # 15 minutes
                'retention_hours': 168    # 7 days
            },
            'alerts': {
                'enabled': True
            }
        }
        
        try:
            if os.path.exists(config_file):
                with open(config_file, 'r') as f:
                    config = yaml.safe_load(f)
                    # Merge with defaults
                    for key, value in default_config.items():
                        if key not in config:
                            config[key] = value
                    return config
            else:
                # Create default config file
                with open(config_file, 'w') as f:
                    yaml.dump(default_config, f, default_flow_style=False)
                return default_config
        except Exception as e:
            logger.error(f"Failed to load config, using defaults: {e}")
            return default_config
    
    def collect_metrics(self) -> InfrastructureHealth:
        """Collect all infrastructure metrics"""
        logger.info("Collecting infrastructure metrics...")
        
        # GitHub Actions metrics
        queue_length = self.github_monitor.get_queue_length()
        success_rate = self.github_monitor.get_success_rate()
        runner_capacity = self.github_monitor.get_runner_capacity()
        
        # Storage metrics
        storage_info = self.storage_monitor.get_storage_usage()
        
        # Registry metrics
        registry_health = self.registry_monitor.check_registry_health()
        
        # Store metrics
        timestamp = datetime.utcnow()
        self.store_metric('build_queue_length', queue_length, timestamp)
        self.store_metric('test_success_rate', success_rate, timestamp, '%')
        self.store_metric('artifact_storage_usage', storage_info['usage_percentage'], timestamp, '%')
        
        # Check alerts
        if self.config['alerts']['enabled']:
            self.alert_manager.check_metric('build_queue_length', queue_length)
            self.alert_manager.check_metric('test_success_rate', success_rate)
            self.alert_manager.check_metric('artifact_storage_usage', storage_info['usage_percentage'])
        
        # Determine overall status
        overall_status = self.calculate_overall_status(
            runner_capacity['status'],
            storage_info['status'],
            registry_health['status'],
            success_rate
        )
        
        # Create health snapshot
        health = InfrastructureHealth(
            timestamp=timestamp,
            github_actions_status=runner_capacity['status'],
            build_queue_length=queue_length,
            test_success_rate=success_rate,
            deployment_status='healthy',  # Would be determined by deployment pipeline
            artifact_storage_usage=storage_info['usage_percentage'],
            container_registry_status=registry_health['status'],
            alerts_active=len(self.alert_manager.active_alerts),
            overall_status=overall_status
        )
        
        logger.info(f"Metrics collected - Status: {overall_status}, Queue: {queue_length}, Success Rate: {success_rate:.1f}%")
        return health
    
    def store_metric(self, name: str, value: float, timestamp: datetime, unit: str = ""):
        """Store a metric value"""
        if name not in self.metrics:
            self.metrics[name] = []
        
        metric = MetricValue(value=value, timestamp=timestamp, unit=unit)
        self.metrics[name].append(metric)
        
        # Clean old metrics
        cutoff = timestamp - timedelta(hours=self.config['monitoring']['retention_hours'])
        self.metrics[name] = [m for m in self.metrics[name] if m.timestamp > cutoff]
    
    def calculate_overall_status(self, *statuses) -> str:
        """Calculate overall infrastructure status"""
        status_priority = {'critical': 0, 'unhealthy': 1, 'degraded': 2, 'warning': 3, 'healthy': 4}
        
        # Special handling for success rate
        success_rate = None
        filtered_statuses = []
        for status in statuses:
            if isinstance(status, (int, float)):
                success_rate = status
            else:
                filtered_statuses.append(status)
        
        # Check success rate
        if success_rate is not None and success_rate < 50:
            filtered_statuses.append('critical')
        elif success_rate is not None and success_rate < 80:
            filtered_statuses.append('warning')
        
        if not filtered_statuses:
            return 'healthy'
        
        # Return worst status
        worst_status = min(filtered_statuses, key=lambda s: status_priority.get(s, 999))
        return worst_status
    
    def generate_dashboard(self, output_file: str = '/tmp/infrastructure-dashboard.html'):
        """Generate HTML dashboard"""
        logger.info(f"Generating infrastructure dashboard: {output_file}")
        
        # Collect current metrics
        health = self.collect_metrics()
        
        # Generate HTML dashboard
        html_content = self.create_dashboard_html(health)
        
        try:
            with open(output_file, 'w') as f:
                f.write(html_content)
            logger.info(f"Dashboard generated: {output_file}")
        except Exception as e:
            logger.error(f"Failed to generate dashboard: {e}")
    
    def create_dashboard_html(self, health: InfrastructureHealth) -> str:
        """Create HTML dashboard content"""
        status_colors = {
            'healthy': '#28a745',
            'warning': '#ffc107',
            'degraded': '#fd7e14',
            'critical': '#dc3545',
            'unhealthy': '#dc3545'
        }
        
        overall_color = status_colors.get(health.overall_status, '#6c757d')
        
        return f"""
<!DOCTYPE html>
<html>
<head>
    <title>RaeenOS Infrastructure Dashboard</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {{ font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; margin: 20px; background: #f8f9fa; }}
        .container {{ max-width: 1200px; margin: 0 auto; }}
        .header {{ text-align: center; margin-bottom: 30px; }}
        .status-badge {{ display: inline-block; padding: 8px 16px; border-radius: 20px; color: white; font-weight: bold; }}
        .metrics-grid {{ display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }}
        .metric-card {{ background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }}
        .metric-value {{ font-size: 2em; font-weight: bold; color: #495057; }}
        .metric-label {{ color: #6c757d; margin-top: 5px; }}
        .alert-section {{ margin-top: 30px; padding: 20px; background: white; border-radius: 8px; }}
        .alert-item {{ padding: 10px; margin: 5px 0; border-left: 4px solid #dc3545; background: #f8f9fa; }}
        .timestamp {{ text-align: center; color: #6c757d; margin-top: 20px; }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>RaeenOS Infrastructure Dashboard</h1>
            <div class="status-badge" style="background-color: {overall_color};">
                Overall Status: {health.overall_status.upper()}
            </div>
        </div>
        
        <div class="metrics-grid">
            <div class="metric-card">
                <div class="metric-value">{health.build_queue_length}</div>
                <div class="metric-label">Build Queue Length</div>
            </div>
            
            <div class="metric-card">
                <div class="metric-value">{health.test_success_rate:.1f}%</div>
                <div class="metric-label">Test Success Rate (24h)</div>
            </div>
            
            <div class="metric-card">
                <div class="metric-value">{health.artifact_storage_usage:.1f}%</div>
                <div class="metric-label">Artifact Storage Usage</div>
            </div>
            
            <div class="metric-card">
                <div class="metric-value" style="color: {status_colors.get(health.github_actions_status, '#6c757d')}">{health.github_actions_status.upper()}</div>
                <div class="metric-label">GitHub Actions Status</div>
            </div>
            
            <div class="metric-card">
                <div class="metric-value" style="color: {status_colors.get(health.container_registry_status, '#6c757d')}">{health.container_registry_status.upper()}</div>
                <div class="metric-label">Container Registry Status</div>
            </div>
            
            <div class="metric-card">
                <div class="metric-value">{health.alerts_active}</div>
                <div class="metric-label">Active Alerts</div>
            </div>
        </div>
        
        <div class="alert-section">
            <h3>Active Alerts</h3>
            {''.join([f'<div class="alert-item">Alert: {alert.metric_name} - {alert.severity}</div>' for alert in self.alert_manager.active_alerts.values()]) or '<p>No active alerts</p>'}
        </div>
        
        <div class="timestamp">
            Last updated: {health.timestamp.strftime('%Y-%m-%d %H:%M:%S UTC')}
        </div>
    </div>
</body>
</html>
        """
    
    def run_monitoring_cycle(self):
        """Run a single monitoring cycle"""
        try:
            health = self.collect_metrics()
            
            # Generate dashboard
            self.generate_dashboard()
            
            # Output health summary
            print(json.dumps(asdict(health), default=str, indent=2))
            
        except Exception as e:
            logger.error(f"Monitoring cycle failed: {e}")
    
    def run_continuous_monitoring(self):
        """Run continuous monitoring loop"""
        logger.info(f"Starting continuous monitoring (interval: {self.config['monitoring']['interval_seconds']}s)")
        
        while True:
            try:
                self.run_monitoring_cycle()
                time.sleep(self.config['monitoring']['interval_seconds'])
            except KeyboardInterrupt:
                logger.info("Monitoring stopped by user")
                break
            except Exception as e:
                logger.error(f"Monitoring error: {e}")
                time.sleep(60)  # Wait 1 minute before retrying

def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description='RaeenOS Infrastructure Monitor')
    parser.add_argument('--config', default='monitor-config.yaml', help='Configuration file')
    parser.add_argument('--once', action='store_true', help='Run once instead of continuously')
    parser.add_argument('--dashboard', help='Generate dashboard to specified file')
    
    args = parser.parse_args()
    
    # Create monitor
    monitor = InfrastructureMonitor(args.config)
    
    if args.dashboard:
        monitor.generate_dashboard(args.dashboard)
        print(f"Dashboard generated: {args.dashboard}")
    elif args.once:
        monitor.run_monitoring_cycle()
    else:
        monitor.run_continuous_monitoring()

if __name__ == '__main__':
    main()