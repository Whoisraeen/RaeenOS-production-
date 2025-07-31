#!/usr/bin/env python3
"""
RaeenOS Integration Test Framework
Cross-agent integration testing for 42-agent development
Version: 1.0
Author: Testing & QA Automation Lead
"""

import json
import time
import subprocess
import sys
import argparse
import os
import threading
import queue
from pathlib import Path
from typing import Dict, List, Any, Optional, Tuple
from dataclasses import dataclass, asdict
from enum import Enum
import yaml
import tempfile
import shutil

class TestStatus(Enum):
    PENDING = "pending"
    RUNNING = "running"
    PASSED = "passed"
    FAILED = "failed"
    SKIPPED = "skipped"
    ERROR = "error"

@dataclass
class IntegrationTestCase:
    """Represents a single integration test case"""
    test_id: str
    name: str
    description: str
    primary_agent: str
    secondary_agents: List[str]
    test_type: str  # "interface", "data_flow", "performance", "security"
    setup_commands: List[str]
    test_commands: List[str]
    cleanup_commands: List[str]
    expected_results: Dict[str, Any]
    timeout_seconds: int = 300
    
    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)

@dataclass
class IntegrationTestResult:
    """Result of an integration test"""
    test_case: IntegrationTestCase
    status: TestStatus
    start_time: float
    end_time: float
    duration: float
    stdout: str
    stderr: str
    return_code: int
    metrics: Dict[str, Any]
    error_message: Optional[str] = None
    
    def to_dict(self) -> Dict[str, Any]:
        result = asdict(self)
        result['test_case'] = self.test_case.to_dict()
        result['status'] = self.status.value
        return result

@dataclass
class IntegrationTestSuite:
    """Collection of integration tests for agent interactions"""
    suite_id: str
    name: str
    description: str
    primary_agent: str
    test_cases: List[IntegrationTestCase]
    setup_commands: List[str]
    cleanup_commands: List[str]
    
    def to_dict(self) -> Dict[str, Any]:
        result = asdict(self)
        result['test_cases'] = [tc.to_dict() for tc in self.test_cases]
        return result

class IntegrationTestExecutor:
    """Executes integration tests between agents"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.test_configs_dir = self.project_root / "tools" / "quality-gates" / "config" / "integration"
        self.test_results_dir = self.project_root / "tools" / "quality-gates" / "reports" / "integration"
        self.temp_dir = None
        
        # Ensure directories exist
        self.test_results_dir.mkdir(parents=True, exist_ok=True)
        
        # Load agent interface definitions
        self.agent_interfaces = self._load_agent_interfaces()
        
        # Integration test matrix
        self.integration_matrix = self._load_integration_matrix()
    
    def _load_agent_interfaces(self) -> Dict[str, Any]:
        """Load agent interface definitions"""
        interfaces_file = self.project_root / "docs" / "INTERFACE_INTEGRATION_GUIDE.md"
        
        # In a real implementation, this would parse the interface definitions
        # For now, we'll return a mock structure
        return {
            "kernel-architect": {
                "provides": ["memory_management", "process_scheduling", "syscall_interface"],
                "requires": ["hardware_abstraction"],
                "interfaces": {
                    "memory_management": {
                        "functions": ["allocate_memory", "free_memory", "get_memory_stats"],
                        "data_structures": ["memory_block", "allocation_info"]
                    }
                }
            },
            "driver-integration-specialist": {
                "provides": ["device_management", "driver_loading"],
                "requires": ["kernel_services"],
                "interfaces": {
                    "device_management": {
                        "functions": ["register_device", "unregister_device", "enumerate_devices"],
                        "data_structures": ["device_info", "driver_context"]
                    }
                }
            }
            # Additional agent interfaces would be defined here
        }
    
    def _load_integration_matrix(self) -> Dict[str, List[str]]:
        """Load integration dependency matrix"""
        matrix_file = self.test_configs_dir / "integration_matrix.yml"
        
        if matrix_file.exists():
            with open(matrix_file, 'r') as f:
                return yaml.safe_load(f)
        
        # Default integration matrix
        return {
            "kernel-architect": ["memory-manager", "hardware-compat-expert", "privacy-security-engineer"],
            "driver-integration-specialist": ["kernel-architect", "hardware-compat-expert", "gaming-layer-engineer"],
            "gaming-layer-engineer": ["driver-integration-specialist", "ux-wizard", "multitasking-maestro"],
            "ux-wizard": ["gaming-layer-engineer", "multitasking-maestro", "brand-identity-guru"],
            "raeen-studio-lead": ["ai-orchestrator", "cloud-integration-engineer", "filesystem-engineer"],
            "ai-orchestrator": ["raeen-studio-lead", "shell-cli-engineer", "privacy-security-engineer"]
        }
    
    def generate_integration_tests(self, primary_agent: str) -> IntegrationTestSuite:
        """Generate integration tests for a primary agent"""
        
        related_agents = self.integration_matrix.get(primary_agent, [])
        
        if not related_agents:
            print(f"Warning: No integration dependencies found for {primary_agent}")
            return IntegrationTestSuite(
                suite_id=f"{primary_agent}_integration",
                name=f"{primary_agent} Integration Tests",
                description=f"Integration tests for {primary_agent}",
                primary_agent=primary_agent,
                test_cases=[],
                setup_commands=[],
                cleanup_commands=[]
            )
        
        test_cases = []
        
        # Generate interface validation tests
        for secondary_agent in related_agents:
            test_cases.extend(self._generate_interface_tests(primary_agent, secondary_agent))
            test_cases.extend(self._generate_data_flow_tests(primary_agent, secondary_agent))
            test_cases.extend(self._generate_performance_tests(primary_agent, secondary_agent))
            test_cases.extend(self._generate_security_tests(primary_agent, secondary_agent))
        
        return IntegrationTestSuite(
            suite_id=f"{primary_agent}_integration",
            name=f"{primary_agent} Integration Tests",
            description=f"Integration tests for {primary_agent} with {len(related_agents)} related agents",
            primary_agent=primary_agent,
            test_cases=test_cases,
            setup_commands=self._get_setup_commands(primary_agent),
            cleanup_commands=self._get_cleanup_commands(primary_agent)
        )
    
    def _generate_interface_tests(self, primary_agent: str, secondary_agent: str) -> List[IntegrationTestCase]:
        """Generate interface validation tests between two agents"""
        
        test_cases = []
        
        # API Contract Validation Test
        test_cases.append(IntegrationTestCase(
            test_id=f"{primary_agent}_{secondary_agent}_api_contract",
            name=f"API Contract Validation: {primary_agent} ↔ {secondary_agent}",
            description=f"Validate API contracts between {primary_agent} and {secondary_agent}",
            primary_agent=primary_agent,
            secondary_agents=[secondary_agent],
            test_type="interface",
            setup_commands=[
                f"load_agent_module {primary_agent}",
                f"load_agent_module {secondary_agent}",
                "initialize_test_environment"
            ],
            test_commands=[
                f"validate_interface_contract {primary_agent} {secondary_agent}",
                f"test_api_endpoints {primary_agent} {secondary_agent}",
                f"verify_data_structures {primary_agent} {secondary_agent}"
            ],
            cleanup_commands=[
                f"unload_agent_module {secondary_agent}",
                f"unload_agent_module {primary_agent}",
                "cleanup_test_environment"
            ],
            expected_results={
                "contract_validation": "pass",
                "api_endpoints_tested": "> 0",
                "data_structure_compatibility": "100%"
            },
            timeout_seconds=120
        ))
        
        # Error Handling Test
        test_cases.append(IntegrationTestCase(
            test_id=f"{primary_agent}_{secondary_agent}_error_handling",
            name=f"Error Handling: {primary_agent} ↔ {secondary_agent}",
            description=f"Test error propagation and handling between {primary_agent} and {secondary_agent}",
            primary_agent=primary_agent,
            secondary_agents=[secondary_agent],
            test_type="interface",
            setup_commands=[
                f"load_agent_module {primary_agent}",
                f"load_agent_module {secondary_agent}",
                "enable_error_injection"
            ],
            test_commands=[
                f"inject_error {secondary_agent} network_failure",
                f"test_error_handling {primary_agent}",
                f"inject_error {primary_agent} resource_exhaustion",
                f"test_error_recovery {secondary_agent}"
            ],
            cleanup_commands=[
                "disable_error_injection",
                f"unload_agent_module {secondary_agent}",
                f"unload_agent_module {primary_agent}"
            ],
            expected_results={
                "error_handling": "graceful",
                "system_stability": "maintained",
                "recovery_time": "< 5 seconds"
            },
            timeout_seconds=180
        ))
        
        return test_cases
    
    def _generate_data_flow_tests(self, primary_agent: str, secondary_agent: str) -> List[IntegrationTestCase]:
        """Generate data flow validation tests"""
        
        test_cases = []
        
        # Data Integrity Test
        test_cases.append(IntegrationTestCase(
            test_id=f"{primary_agent}_{secondary_agent}_data_integrity",
            name=f"Data Integrity: {primary_agent} → {secondary_agent}",
            description=f"Validate data integrity in communication between {primary_agent} and {secondary_agent}",
            primary_agent=primary_agent,
            secondary_agents=[secondary_agent],
            test_type="data_flow",
            setup_commands=[
                f"load_agent_module {primary_agent}",
                f"load_agent_module {secondary_agent}",
                "setup_data_integrity_monitoring"
            ],
            test_commands=[
                "generate_test_data_with_checksum",
                f"send_data {primary_agent} {secondary_agent}",
                f"verify_data_integrity {secondary_agent}",
                "validate_checksums"
            ],
            cleanup_commands=[
                "cleanup_test_data",
                f"unload_agent_module {secondary_agent}",
                f"unload_agent_module {primary_agent}"
            ],
            expected_results={
                "data_corruption": "0%",
                "checksum_validation": "100%",
                "data_transmission": "complete"
            },
            timeout_seconds=150
        ))
        
        # Concurrent Access Test
        test_cases.append(IntegrationTestCase(
            test_id=f"{primary_agent}_{secondary_agent}_concurrent_access",
            name=f"Concurrent Access: {primary_agent} ↔ {secondary_agent}",
            description=f"Test concurrent data access between {primary_agent} and {secondary_agent}",
            primary_agent=primary_agent,
            secondary_agents=[secondary_agent],
            test_type="data_flow",
            setup_commands=[
                f"load_agent_module {primary_agent}",
                f"load_agent_module {secondary_agent}",
                "setup_concurrent_test_environment"
            ],
            test_commands=[
                "start_concurrent_data_access_test",
                f"simulate_high_load {primary_agent}",
                f"simulate_high_load {secondary_agent}",
                "monitor_race_conditions",
                "verify_data_consistency"
            ],
            cleanup_commands=[
                "stop_concurrent_test",
                "cleanup_concurrent_environment",
                f"unload_agent_module {secondary_agent}",
                f"unload_agent_module {primary_agent}"
            ],
            expected_results={
                "race_conditions": "0",
                "deadlocks": "0",
                "data_consistency": "100%"
            },
            timeout_seconds=300
        ))
        
        return test_cases
    
    def _generate_performance_tests(self, primary_agent: str, secondary_agent: str) -> List[IntegrationTestCase]:
        """Generate performance integration tests"""
        
        test_cases = []
        
        # Latency Test
        test_cases.append(IntegrationTestCase(
            test_id=f"{primary_agent}_{secondary_agent}_latency",
            name=f"Communication Latency: {primary_agent} ↔ {secondary_agent}",
            description=f"Measure communication latency between {primary_agent} and {secondary_agent}",
            primary_agent=primary_agent,
            secondary_agents=[secondary_agent],
            test_type="performance",
            setup_commands=[
                f"load_agent_module {primary_agent}",
                f"load_agent_module {secondary_agent}",
                "setup_latency_monitoring"
            ],
            test_commands=[
                "start_latency_measurement",
                f"send_ping_requests {primary_agent} {secondary_agent} 1000",
                "calculate_latency_statistics",
                "verify_latency_targets"
            ],
            cleanup_commands=[
                "stop_latency_monitoring",
                f"unload_agent_module {secondary_agent}",
                f"unload_agent_module {primary_agent}"
            ],
            expected_results={
                "avg_latency": "< 10ms",
                "max_latency": "< 50ms",
                "latency_jitter": "< 5ms"
            },
            timeout_seconds=180
        ))
        
        # Throughput Test
        test_cases.append(IntegrationTestCase(
            test_id=f"{primary_agent}_{secondary_agent}_throughput",
            name=f"Data Throughput: {primary_agent} ↔ {secondary_agent}",
            description=f"Measure data throughput between {primary_agent} and {secondary_agent}",
            primary_agent=primary_agent,
            secondary_agents=[secondary_agent],
            test_type="performance",
            setup_commands=[
                f"load_agent_module {primary_agent}",
                f"load_agent_module {secondary_agent}",
                "setup_throughput_monitoring"
            ],
            test_commands=[
                "start_throughput_measurement",
                f"transfer_large_dataset {primary_agent} {secondary_agent}",
                "calculate_throughput_statistics",
                "verify_throughput_targets"
            ],
            cleanup_commands=[
                "stop_throughput_monitoring",
                "cleanup_test_data",
                f"unload_agent_module {secondary_agent}",
                f"unload_agent_module {primary_agent}"
            ],
            expected_results={
                "throughput": "> 100 MB/s",
                "cpu_utilization": "< 80%",
                "memory_utilization": "< 75%"
            },
            timeout_seconds=240
        ))
        
        return test_cases
    
    def _generate_security_tests(self, primary_agent: str, secondary_agent: str) -> List[IntegrationTestCase]:
        """Generate security integration tests"""
        
        test_cases = []
        
        # Access Control Test
        test_cases.append(IntegrationTestCase(
            test_id=f"{primary_agent}_{secondary_agent}_access_control",
            name=f"Access Control: {primary_agent} ↔ {secondary_agent}",
            description=f"Validate access control between {primary_agent} and {secondary_agent}",
            primary_agent=primary_agent,
            secondary_agents=[secondary_agent],
            test_type="security",
            setup_commands=[
                f"load_agent_module {primary_agent}",
                f"load_agent_module {secondary_agent}",
                "setup_security_monitoring"
            ],
            test_commands=[
                "configure_access_restrictions",
                f"test_authorized_access {primary_agent} {secondary_agent}",
                f"test_unauthorized_access {primary_agent} {secondary_agent}",
                "verify_access_control_enforcement"
            ],
            cleanup_commands=[
                "cleanup_security_configuration",
                f"unload_agent_module {secondary_agent}",
                f"unload_agent_module {primary_agent}"
            ],
            expected_results={
                "authorized_access": "allowed",
                "unauthorized_access": "blocked",
                "security_violations": "0"
            },
            timeout_seconds=120
        ))
        
        # Data Encryption Test
        test_cases.append(IntegrationTestCase(
            test_id=f"{primary_agent}_{secondary_agent}_encryption",
            name=f"Data Encryption: {primary_agent} ↔ {secondary_agent}",
            description=f"Validate data encryption in communication between {primary_agent} and {secondary_agent}",
            primary_agent=primary_agent,
            secondary_agents=[secondary_agent],
            test_type="security",
            setup_commands=[
                f"load_agent_module {primary_agent}",
                f"load_agent_module {secondary_agent}",
                "setup_encryption_monitoring"
            ],
            test_commands=[
                "enable_encryption",
                f"send_encrypted_data {primary_agent} {secondary_agent}",
                "verify_encryption_in_transit",
                "test_decryption_integrity"
            ],
            cleanup_commands=[
                "disable_encryption",
                f"unload_agent_module {secondary_agent}",
                f"unload_agent_module {primary_agent}"
            ],
            expected_results={
                "encryption_enabled": "true",
                "data_encrypted": "100%",
                "decryption_integrity": "100%"
            },
            timeout_seconds=150
        ))
        
        return test_cases
    
    def _get_setup_commands(self, agent: str) -> List[str]:
        """Get setup commands for agent-specific environment"""
        
        base_commands = [
            "initialize_test_environment",
            "setup_logging",
            "create_temp_workspace"
        ]
        
        agent_specific = {
            "kernel-architect": [
                "setup_kernel_test_environment",
                "load_test_kernel_modules",
                "initialize_memory_subsystem"
            ],
            "gaming-layer-engineer": [
                "setup_graphics_test_environment",
                "initialize_virtual_display",
                "load_test_gpu_drivers"
            ],
            "raeen-studio-lead": [
                "setup_ui_test_environment",
                "initialize_test_database",
                "start_test_services"
            ]
        }
        
        return base_commands + agent_specific.get(agent, [])
    
    def _get_cleanup_commands(self, agent: str) -> List[str]:
        """Get cleanup commands for agent-specific environment"""
        
        base_commands = [
            "cleanup_temp_workspace",
            "stop_logging",
            "cleanup_test_environment"
        ]
        
        agent_specific = {
            "kernel-architect": [
                "unload_test_kernel_modules",
                "cleanup_memory_subsystem",
                "restore_kernel_state"
            ],
            "gaming-layer-engineer": [
                "unload_test_gpu_drivers",
                "cleanup_virtual_display",
                "restore_graphics_state"
            ],
            "raeen-studio-lead": [
                "stop_test_services",
                "cleanup_test_database",
                "restore_ui_state"
            ]
        }
        
        return agent_specific.get(agent, []) + base_commands
    
    def execute_test_case(self, test_case: IntegrationTestCase) -> IntegrationTestResult:
        """Execute a single integration test case"""
        
        print(f"  Executing: {test_case.name}")
        
        start_time = time.time()
        stdout_lines = []
        stderr_lines = []
        return_code = 0
        error_message = None
        metrics = {}
        
        try:
            # Create temporary working directory
            with tempfile.TemporaryDirectory() as temp_dir:
                self.temp_dir = Path(temp_dir)
                
                # Execute setup commands
                for cmd in test_case.setup_commands:
                    result = self._execute_command(cmd, temp_dir)
                    if result.returncode != 0:
                        error_message = f"Setup command failed: {cmd}"
                        return_code = result.returncode
                        break
                
                # Execute test commands if setup succeeded
                if return_code == 0:
                    for cmd in test_case.test_commands:
                        result = self._execute_command(cmd, temp_dir)
                        stdout_lines.append(result.stdout)
                        stderr_lines.append(result.stderr)
                        
                        if result.returncode != 0:
                            error_message = f"Test command failed: {cmd}"
                            return_code = result.returncode
                            break
                
                # Always execute cleanup commands
                for cmd in test_case.cleanup_commands:
                    try:
                        self._execute_command(cmd, temp_dir)
                    except Exception as e:
                        print(f"Warning: Cleanup command failed: {cmd} - {e}")
                
                # Parse metrics from stdout
                metrics = self._parse_test_metrics('\n'.join(stdout_lines))
                
        except Exception as e:
            error_message = f"Test execution error: {str(e)}"
            return_code = 1
        
        end_time = time.time()
        
        # Determine test status
        if return_code == 0 and self._validate_expected_results(test_case, metrics):
            status = TestStatus.PASSED
        elif return_code != 0:
            status = TestStatus.FAILED
        else:
            status = TestStatus.FAILED
            if not error_message:
                error_message = "Expected results validation failed"
        
        return IntegrationTestResult(
            test_case=test_case,
            status=status,
            start_time=start_time,
            end_time=end_time,
            duration=end_time - start_time,
            stdout='\n'.join(stdout_lines),
            stderr='\n'.join(stderr_lines),
            return_code=return_code,
            metrics=metrics,
            error_message=error_message
        )
    
    def _execute_command(self, command: str, working_dir: str) -> subprocess.CompletedProcess:
        """Execute a test command"""
        
        # In a real implementation, this would map test commands to actual executables
        # For demonstration, we'll simulate command execution
        
        if command.startswith("load_agent_module"):
            # Simulate loading an agent module
            time.sleep(0.1)
            return subprocess.CompletedProcess(
                args=[command], returncode=0, stdout="Module loaded successfully", stderr=""
            )
        elif command.startswith("validate_interface_contract"):
            # Simulate interface validation
            time.sleep(0.5)
            return subprocess.CompletedProcess(
                args=[command], returncode=0, 
                stdout="contract_validation=pass\napi_endpoints_tested=15\ndata_structure_compatibility=100%", 
                stderr=""
            )
        elif command.startswith("test_error_handling"):
            # Simulate error handling test
            time.sleep(0.3)
            return subprocess.CompletedProcess(
                args=[command], returncode=0,
                stdout="error_handling=graceful\nsystem_stability=maintained\nrecovery_time=2.3",
                stderr=""
            )
        elif command.startswith("send_ping_requests"):
            # Simulate latency test
            time.sleep(1.0)
            return subprocess.CompletedProcess(
                args=[command], returncode=0,
                stdout="avg_latency=7.2\nmax_latency=15.8\nlatency_jitter=2.1",
                stderr=""
            )
        else:
            # Default simulation
            time.sleep(0.1)
            return subprocess.CompletedProcess(
                args=[command], returncode=0, stdout=f"Command executed: {command}", stderr=""
            )
    
    def _parse_test_metrics(self, stdout: str) -> Dict[str, Any]:
        """Parse metrics from test output"""
        
        metrics = {}
        
        for line in stdout.split('\n'):
            if '=' in line and not line.startswith('Command executed'):
                try:
                    key, value = line.split('=', 1)
                    key = key.strip()
                    value = value.strip()
                    
                    # Try to convert to appropriate type
                    try:
                        metrics[key] = float(value)
                    except ValueError:
                        metrics[key] = value
                except ValueError:
                    continue
        
        return metrics
    
    def _validate_expected_results(self, test_case: IntegrationTestCase, metrics: Dict[str, Any]) -> bool:
        """Validate test results against expected outcomes"""
        
        for expected_key, expected_value in test_case.expected_results.items():
            if expected_key not in metrics:
                print(f"    Missing expected metric: {expected_key}")
                return False
            
            actual_value = metrics[expected_key]
            
            # Handle different comparison types
            if isinstance(expected_value, str):
                if expected_value.startswith('>'):
                    threshold = float(expected_value[1:].strip())
                    if not (isinstance(actual_value, (int, float)) and actual_value > threshold):
                        print(f"    Metric {expected_key}: {actual_value} not > {threshold}")
                        return False
                elif expected_value.startswith('<'):
                    threshold = float(expected_value[1:].strip())
                    if not (isinstance(actual_value, (int, float)) and actual_value < threshold):
                        print(f"    Metric {expected_key}: {actual_value} not < {threshold}")
                        return False
                elif expected_value != str(actual_value):
                    print(f"    Metric {expected_key}: {actual_value} != {expected_value}")
                    return False
            elif expected_value != actual_value:
                print(f"    Metric {expected_key}: {actual_value} != {expected_value}")
                return False
        
        return True
    
    def execute_test_suite(self, test_suite: IntegrationTestSuite) -> Dict[str, Any]:
        """Execute a complete integration test suite"""
        
        print(f"Executing integration test suite: {test_suite.name}")
        print(f"Primary agent: {test_suite.primary_agent}")
        print(f"Test cases: {len(test_suite.test_cases)}")
        
        suite_start_time = time.time()
        results = []
        
        # Execute all test cases
        for test_case in test_suite.test_cases:
            result = self.execute_test_case(test_case)
            results.append(result)
        
        suite_end_time = time.time()
        
        # Generate summary
        passed_count = sum(1 for r in results if r.status == TestStatus.PASSED)
        failed_count = sum(1 for r in results if r.status == TestStatus.FAILED)
        error_count = sum(1 for r in results if r.status == TestStatus.ERROR)
        
        suite_result = {
            "suite": test_suite.to_dict(),
            "summary": {
                "total_tests": len(results),
                "passed": passed_count,
                "failed": failed_count,
                "errors": error_count,
                "success_rate": (passed_count / len(results)) * 100 if results else 0,
                "start_time": suite_start_time,
                "end_time": suite_end_time,
                "duration": suite_end_time - suite_start_time
            },
            "results": [r.to_dict() for r in results]
        }
        
        return suite_result

def main():
    parser = argparse.ArgumentParser(description="RaeenOS Integration Test Framework")
    parser.add_argument("--primary-agent", required=True, help="Primary agent to test")
    parser.add_argument("--related-agents", help="Comma-separated list of related agents")
    parser.add_argument("--component-path", default=".", help="Path to component code")
    parser.add_argument("--output", help="Output file for results (JSON format)")
    parser.add_argument("--project-root", default=os.getcwd(), help="Project root directory")
    
    args = parser.parse_args()
    
    # Create test executor
    executor = IntegrationTestExecutor(args.project_root)
    
    # Generate test suite
    test_suite = executor.generate_integration_tests(args.primary_agent)
    
    if not test_suite.test_cases:
        print(f"No integration tests generated for {args.primary_agent}")
        sys.exit(0)
    
    # Execute test suite
    suite_result = executor.execute_test_suite(test_suite)
    
    # Output results
    result_json = json.dumps(suite_result, indent=2)
    
    if args.output:
        with open(args.output, 'w') as f:
            f.write(result_json)
        print(f"\nResults written to {args.output}")
    else:
        print("\nIntegration Test Results:")
        print(json.dumps(suite_result['summary'], indent=2))
    
    # Print summary
    summary = suite_result['summary']
    print(f"\nIntegration Test Summary for {args.primary_agent}:")
    print(f"  Total tests: {summary['total_tests']}")
    print(f"  Passed: {summary['passed']}")
    print(f"  Failed: {summary['failed']}")
    print(f"  Errors: {summary['errors']}")
    print(f"  Success rate: {summary['success_rate']:.1f}%")
    print(f"  Duration: {summary['duration']:.2f} seconds")
    
    # Exit with appropriate code
    if summary['failed'] > 0 or summary['errors'] > 0:
        print(f"\nIntegration tests FAILED for {args.primary_agent}")
        sys.exit(1)
    else:
        print(f"\nIntegration tests PASSED for {args.primary_agent}")
        sys.exit(0)

if __name__ == "__main__":
    main()