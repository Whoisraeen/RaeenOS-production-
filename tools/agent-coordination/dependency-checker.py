#!/usr/bin/env python3
"""
RaeenOS Agent Dependency Checker
Analyzes cross-component dependencies and validates interface compatibility
between different RaeenOS development agents
"""

import json
import os
import re
import sys
import yaml
import logging
from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass, field
from pathlib import Path
import subprocess

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

@dataclass
class InterfaceDefinition:
    """Represents an interface definition"""
    name: str
    file_path: str
    functions: List[str] = field(default_factory=list)
    structures: List[str] = field(default_factory=list)
    constants: List[str] = field(default_factory=list)
    version: str = "1.0.0"
    dependencies: List[str] = field(default_factory=list)

@dataclass
class AgentComponent:
    """Represents a component owned by an agent"""
    name: str
    agent_type: str
    file_paths: List[str] = field(default_factory=list)
    interfaces_provided: List[str] = field(default_factory=list)
    interfaces_required: List[str] = field(default_factory=list)
    dependencies: List[str] = field(default_factory=list)

@dataclass
class DependencyIssue:
    """Represents a dependency validation issue"""
    severity: str  # "error", "warning", "info"
    component: str
    interface: str
    message: str
    file_path: Optional[str] = None
    line_number: Optional[int] = None

class InterfaceParser:
    """Parse interface definitions from header files"""
    
    def __init__(self):
        self.function_pattern = re.compile(r'\s*([a-zA-Z_][a-zA-Z0-9_]*\s+[*]*)\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*;')
        self.struct_pattern = re.compile(r'typedef\s+struct\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*{')
        self.define_pattern = re.compile(r'#define\s+([A-Z_][A-Z0-9_]*)\s+')
        self.include_pattern = re.compile(r'#include\s+[<"]([^>"]+)[>"]')
    
    def parse_header_file(self, file_path: str) -> InterfaceDefinition:
        """Parse a header file and extract interface information"""
        interface_name = Path(file_path).stem
        interface = InterfaceDefinition(name=interface_name, file_path=file_path)
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Extract function declarations
            for match in self.function_pattern.finditer(content):
                function_name = match.group(2)
                interface.functions.append(function_name)
            
            # Extract structure definitions
            for match in self.struct_pattern.finditer(content):
                struct_name = match.group(1)
                interface.structures.append(struct_name)
            
            # Extract constants
            for match in self.define_pattern.finditer(content):
                define_name = match.group(1)
                interface.constants.append(define_name)
            
            # Extract dependencies (includes)
            for match in self.include_pattern.finditer(content):
                include_file = match.group(1)
                if not include_file.startswith('std') and not include_file.startswith('sys/'):
                    interface.dependencies.append(include_file)
            
            logger.debug(f"Parsed interface {interface_name}: {len(interface.functions)} functions, "
                        f"{len(interface.structures)} structures, {len(interface.constants)} constants")
            
        except Exception as e:
            logger.error(f"Failed to parse {file_path}: {e}")
        
        return interface

class ComponentAnalyzer:
    """Analyze components and their dependencies"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.interface_parser = InterfaceParser()
        
        # Agent to component mapping based on RaeenOS architecture
        self.agent_components = {
            'kernel-architect': ['kernel/kernel.c', 'kernel/memory.c', 'kernel/process/', 'kernel/include/'],
            'driver-integration-specialist': ['drivers/', 'kernel/driver.c'],
            'hardware-compat-expert': ['drivers/pci/', 'drivers/gpu/', 'drivers/audio/', 'drivers/usb/'],
            'memory-manager': ['kernel/memory.c', 'kernel/pmm.c', 'kernel/paging.c'],
            'filesystem-engineer': ['kernel/fs/', 'kernel/include/filesystem_interface.h'],
            'network-architect': ['kernel/net/', 'drivers/network/'],
            'security-engineer': ['kernel/security/', 'kernel/module_signing.c'],
            'process-scheduler': ['kernel/process/', 'kernel/syscall.c'],
            'ui-designer': ['kernel/ui/', 'kernel/graphics.c', 'kernel/window.c'],
            'package-manager': ['pkg/', 'userland/app_store.c'],
            'testing-qa': ['tests/'],
            'build-system': ['Makefile', 'Makefile.multi-platform'],
        }
        
        # Critical interfaces that must remain stable
        self.critical_interfaces = {
            'memory_interface.h',
            'process_interface.h',
            'filesystem_interface.h',
            'driver_framework.h',
            'syscall.h',
            'hal_interface.h'
        }
    
    def discover_components(self) -> List[AgentComponent]:
        """Discover components from the project structure"""
        components = []
        
        for agent_type, paths in self.agent_components.items():
            component = AgentComponent(name=f"{agent_type}-component", agent_type=agent_type)
            
            for path_pattern in paths:
                full_path = self.project_root / path_pattern
                
                if full_path.is_file():
                    component.file_paths.append(str(full_path))
                elif full_path.is_dir():
                    # Recursively find files in directory
                    for file_path in full_path.rglob('*'):
                        if file_path.is_file() and file_path.suffix in ['.c', '.h', '.cpp', '.hpp']:
                            component.file_paths.append(str(file_path))
            
            if component.file_paths:
                components.append(component)
                logger.debug(f"Discovered component {component.name} with {len(component.file_paths)} files")
        
        return components
    
    def analyze_component_interfaces(self, component: AgentComponent) -> AgentComponent:
        """Analyze interfaces provided and required by a component"""
        for file_path in component.file_paths:
            if not file_path.endswith('.h'):
                continue
            
            # If this is a header file, it likely provides interfaces
            interface_name = Path(file_path).name
            component.interfaces_provided.append(interface_name)
            
            # Parse the file to find dependencies
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                
                # Find include statements to determine required interfaces
                include_pattern = re.compile(r'#include\s+[<"]([^>"]+\.h)[>"]')
                for match in include_pattern.finditer(content):
                    included_file = match.group(1)
                    # Only consider local includes (not system headers)
                    if not included_file.startswith('std') and '/' in included_file:
                        component.interfaces_required.append(Path(included_file).name)
                
            except Exception as e:
                logger.warning(f"Could not analyze {file_path}: {e}")
        
        # Remove duplicates
        component.interfaces_provided = list(set(component.interfaces_provided))
        component.interfaces_required = list(set(component.interfaces_required))
        
        return component

class DependencyValidator:
    """Validate dependencies between components"""
    
    def __init__(self):
        self.issues: List[DependencyIssue] = []
    
    def validate_interface_compatibility(self, components: List[AgentComponent]) -> List[DependencyIssue]:
        """Validate interface compatibility between components"""
        self.issues = []
        
        # Build interface provider map
        interface_providers = {}
        for component in components:
            for interface in component.interfaces_provided:
                if interface not in interface_providers:
                    interface_providers[interface] = []
                interface_providers[interface].append(component.name)
        
        # Check for missing interfaces
        for component in components:
            for required_interface in component.interfaces_required:
                if required_interface not in interface_providers:
                    self.issues.append(DependencyIssue(
                        severity="error",
                        component=component.name,
                        interface=required_interface,
                        message=f"Required interface '{required_interface}' is not provided by any component"
                    ))
        
        # Check for circular dependencies
        self.check_circular_dependencies(components)
        
        # Check for multiple providers of critical interfaces
        self.check_interface_ownership(interface_providers)
        
        return self.issues
    
    def check_circular_dependencies(self, components: List[AgentComponent]):
        """Check for circular dependencies between components"""
        # Build dependency graph
        dependency_graph = {}
        for component in components:
            dependency_graph[component.name] = set()
            
            # Find which components provide the interfaces this component requires
            for required_interface in component.interfaces_required:
                for other_component in components:
                    if (required_interface in other_component.interfaces_provided and 
                        other_component.name != component.name):
                        dependency_graph[component.name].add(other_component.name)
        
        # Use DFS to detect cycles
        visited = set()
        rec_stack = set()
        
        def has_cycle(node):
            if node in rec_stack:
                return True
            if node in visited:
                return False
            
            visited.add(node)
            rec_stack.add(node)
            
            for neighbor in dependency_graph.get(node, set()):
                if has_cycle(neighbor):
                    return True
            
            rec_stack.remove(node)
            return False
        
        for component_name in dependency_graph:
            if component_name not in visited:
                if has_cycle(component_name):
                    self.issues.append(DependencyIssue(
                        severity="error",
                        component=component_name,
                        interface="",
                        message=f"Circular dependency detected involving component '{component_name}'"
                    ))
    
    def check_interface_ownership(self, interface_providers: Dict[str, List[str]]):
        """Check for proper interface ownership"""
        for interface, providers in interface_providers.items():
            if len(providers) > 1:
                # Multiple providers might be okay for some interfaces, but warn for critical ones
                severity = "error" if any(critical in interface for critical in [
                    'memory_interface', 'process_interface', 'filesystem_interface'
                ]) else "warning"
                
                self.issues.append(DependencyIssue(
                    severity=severity,
                    component=", ".join(providers),
                    interface=interface,
                    message=f"Interface '{interface}' is provided by multiple components: {', '.join(providers)}"
                ))

class CompatibilityChecker:
    """Check compatibility between interface versions"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.interface_parser = InterfaceParser()
    
    def check_interface_changes(self, base_commit: str = "HEAD~1") -> List[DependencyIssue]:
        """Check for breaking interface changes since base commit"""
        issues = []
        
        try:
            # Get list of changed header files
            result = subprocess.run([
                'git', 'diff', '--name-only', base_commit, 'HEAD', '--', '*.h'
            ], cwd=self.project_root, capture_output=True, text=True)
            
            changed_files = result.stdout.strip().split('\n') if result.stdout.strip() else []
            
            for file_path in changed_files:
                if not file_path or not file_path.endswith('.h'):
                    continue
                
                full_path = self.project_root / file_path
                if not full_path.exists():
                    continue
                
                # Parse current version
                current_interface = self.interface_parser.parse_header_file(str(full_path))
                
                # Get previous version
                try:
                    result = subprocess.run([
                        'git', 'show', f'{base_commit}:{file_path}'
                    ], cwd=self.project_root, capture_output=True, text=True)
                    
                    if result.returncode == 0:
                        # Write previous version to temp file
                        import tempfile
                        with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
                            f.write(result.stdout)
                            temp_path = f.name
                        
                        try:
                            previous_interface = self.interface_parser.parse_header_file(temp_path)
                            
                            # Compare interfaces
                            breaking_changes = self.compare_interfaces(previous_interface, current_interface)
                            for change in breaking_changes:
                                issues.append(DependencyIssue(
                                    severity="error",
                                    component=Path(file_path).parent.name,
                                    interface=current_interface.name,
                                    message=change,
                                    file_path=file_path
                                ))
                        
                        finally:
                            os.unlink(temp_path)
                
                except Exception as e:
                    logger.warning(f"Could not get previous version of {file_path}: {e}")
        
        except Exception as e:
            logger.error(f"Failed to check interface changes: {e}")
        
        return issues
    
    def compare_interfaces(self, previous: InterfaceDefinition, current: InterfaceDefinition) -> List[str]:
        """Compare two interface definitions and find breaking changes"""
        breaking_changes = []
        
        # Check for removed functions
        removed_functions = set(previous.functions) - set(current.functions)
        for func in removed_functions:
            breaking_changes.append(f"Function '{func}' was removed - this is a breaking change")
        
        # Check for removed structures
        removed_structures = set(previous.structures) - set(current.structures)
        for struct in removed_structures:
            breaking_changes.append(f"Structure '{struct}' was removed - this is a breaking change")
        
        # Check for removed constants
        removed_constants = set(previous.constants) - set(current.constants)
        for const in removed_constants:
            breaking_changes.append(f"Constant '{const}' was removed - this is a breaking change")
        
        return breaking_changes

class DependencyReporter:
    """Generate dependency analysis reports"""
    
    def __init__(self):
        pass
    
    def generate_report(self, components: List[AgentComponent], issues: List[DependencyIssue], 
                       output_format: str = "console") -> str:
        """Generate dependency analysis report"""
        if output_format == "json":
            return self.generate_json_report(components, issues)
        elif output_format == "html":
            return self.generate_html_report(components, issues)
        else:
            return self.generate_console_report(components, issues)
    
    def generate_console_report(self, components: List[AgentComponent], issues: List[DependencyIssue]) -> str:
        """Generate console-formatted report"""
        report = []
        report.append("RaeenOS Dependency Analysis Report")
        report.append("=" * 50)
        report.append("")
        
        # Summary
        error_count = len([i for i in issues if i.severity == "error"])
        warning_count = len([i for i in issues if i.severity == "warning"])
        
        report.append(f"Components analyzed: {len(components)}")
        report.append(f"Issues found: {len(issues)} ({error_count} errors, {warning_count} warnings)")
        report.append("")
        
        # Component overview
        report.append("Component Overview:")
        report.append("-" * 20)
        for component in components:
            report.append(f"• {component.name} ({component.agent_type})")
            report.append(f"  Files: {len(component.file_paths)}")
            report.append(f"  Provides: {len(component.interfaces_provided)} interfaces")
            report.append(f"  Requires: {len(component.interfaces_required)} interfaces")
            report.append("")
        
        # Issues
        if issues:
            report.append("Issues Found:")
            report.append("-" * 15)
            
            for severity in ["error", "warning", "info"]:
                severity_issues = [i for i in issues if i.severity == severity]
                if severity_issues:
                    report.append(f"\n{severity.upper()}S:")
                    for issue in severity_issues:
                        report.append(f"• [{issue.component}] {issue.message}")
                        if issue.interface:
                            report.append(f"  Interface: {issue.interface}")
                        if issue.file_path:
                            report.append(f"  File: {issue.file_path}")
        else:
            report.append("✅ No dependency issues found!")
        
        return "\n".join(report)
    
    def generate_json_report(self, components: List[AgentComponent], issues: List[DependencyIssue]) -> str:
        """Generate JSON-formatted report"""
        report_data = {
            "timestamp": "2025-07-30T12:00:00Z",  # Would be current timestamp
            "summary": {
                "components_analyzed": len(components),
                "total_issues": len(issues),
                "errors": len([i for i in issues if i.severity == "error"]),
                "warnings": len([i for i in issues if i.severity == "warning"])
            },
            "components": [
                {
                    "name": comp.name,
                    "agent_type": comp.agent_type,
                    "file_count": len(comp.file_paths),
                    "interfaces_provided": comp.interfaces_provided,
                    "interfaces_required": comp.interfaces_required
                }
                for comp in components
            ],
            "issues": [
                {
                    "severity": issue.severity,
                    "component": issue.component,
                    "interface": issue.interface,
                    "message": issue.message,
                    "file_path": issue.file_path,
                    "line_number": issue.line_number
                }
                for issue in issues
            ]
        }
        
        return json.dumps(report_data, indent=2)
    
    def generate_html_report(self, components: List[AgentComponent], issues: List[DependencyIssue]) -> str:
        """Generate HTML-formatted report"""
        error_count = len([i for i in issues if i.severity == "error"])
        warning_count = len([i for i in issues if i.severity == "warning"])
        
        return f"""
<!DOCTYPE html>
<html>
<head>
    <title>RaeenOS Dependency Analysis Report</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        .summary {{ background: #f8f9fa; padding: 15px; border-radius: 5px; margin-bottom: 20px; }}
        .component {{ border: 1px solid #ddd; margin: 10px 0; padding: 15px; border-radius: 5px; }}
        .error {{ color: #dc3545; }}
        .warning {{ color: #ffc107; }}
        .info {{ color: #17a2b8; }}
        .issue {{ margin: 10px 0; padding: 10px; border-left: 4px solid #ddd; }}
        .issue.error {{ border-left-color: #dc3545; }}
        .issue.warning {{ border-left-color: #ffc107; }}
        .issue.info {{ border-left-color: #17a2b8; }}
    </style>
</head>
<body>
    <h1>RaeenOS Dependency Analysis Report</h1>
    
    <div class="summary">
        <h3>Summary</h3>
        <p>Components analyzed: {len(components)}</p>
        <p>Issues found: {len(issues)} ({error_count} errors, {warning_count} warnings)</p>
    </div>
    
    <h2>Components</h2>
    {''.join([f'<div class="component"><h4>{comp.name}</h4><p>Agent: {comp.agent_type}</p><p>Files: {len(comp.file_paths)}</p></div>' for comp in components])}
    
    <h2>Issues</h2>
    {''.join([f'<div class="issue {issue.severity}"><strong>{issue.severity.upper()}</strong>: {issue.message}<br><small>Component: {issue.component}, Interface: {issue.interface}</small></div>' for issue in issues]) or '<p>No issues found!</p>'}
</body>
</html>
        """

def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description='RaeenOS Agent Dependency Checker')
    parser.add_argument('--project-root', default='.', help='Project root directory')
    parser.add_argument('--format', choices=['console', 'json', 'html'], default='console', help='Output format')
    parser.add_argument('--output', help='Output file (default: stdout)')
    parser.add_argument('--check-changes', help='Check changes since specified commit')
    parser.add_argument('--verbose', action='store_true', help='Verbose output')
    
    args = parser.parse_args()
    
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    # Initialize analyzers
    analyzer = ComponentAnalyzer(args.project_root)
    validator = DependencyValidator()
    reporter = DependencyReporter()
    
    # Discover and analyze components
    logger.info("Discovering components...")
    components = analyzer.discover_components()
    
    logger.info("Analyzing component interfaces...")
    for i, component in enumerate(components):
        components[i] = analyzer.analyze_component_interfaces(component)
    
    # Validate dependencies
    logger.info("Validating dependencies...")
    issues = validator.validate_interface_compatibility(components)
    
    # Check for interface changes if requested
    if args.check_changes:
        logger.info(f"Checking interface changes since {args.check_changes}...")
        compatibility_checker = CompatibilityChecker(args.project_root)
        change_issues = compatibility_checker.check_interface_changes(args.check_changes)
        issues.extend(change_issues)
    
    # Generate report
    report = reporter.generate_report(components, issues, args.format)
    
    # Output report
    if args.output:
        with open(args.output, 'w') as f:
            f.write(report)
        logger.info(f"Report written to {args.output}")
    else:
        print(report)
    
    # Exit with error code if there are errors
    error_count = len([i for i in issues if i.severity == "error"])
    sys.exit(1 if error_count > 0 else 0)

if __name__ == '__main__':
    main()