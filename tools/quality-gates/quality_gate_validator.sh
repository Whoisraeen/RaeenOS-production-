#!/bin/bash
# RaeenOS Quality Gate Validator
# Automated quality gate execution for 42-agent development
# Version: 1.0
# Author: Testing & QA Automation Lead

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
QUALITY_CONFIG_DIR="$SCRIPT_DIR/config"
REPORTS_DIR="$SCRIPT_DIR/reports"
LOGS_DIR="$SCRIPT_DIR/logs"

# Ensure directories exist
mkdir -p "$REPORTS_DIR" "$LOGS_DIR"

# Quality gate levels
QUALITY_GATE_LEVEL=$1
AGENT_NAME=$2
COMPONENT_PATH=$3

# Logging functions
log_info() { echo "[INFO] $(date '+%Y-%m-%d %H:%M:%S') - $1" | tee -a "$LOGS_DIR/quality_gate.log"; }
log_error() { echo "[ERROR] $(date '+%Y-%m-%d %H:%M:%S') - $1" | tee -a "$LOGS_DIR/quality_gate.log" >&2; }
log_success() { echo "[SUCCESS] $(date '+%Y-%m-%d %H:%M:%S') - $1" | tee -a "$LOGS_DIR/quality_gate.log"; }

# Validation functions
validate_inputs() {
    if [[ -z "$QUALITY_GATE_LEVEL" ]]; then
        log_error "Quality gate level not specified. Usage: $0 <level> <agent> [component_path]"
        echo "Available levels: pre_commit, pre_merge, pre_release"
        exit 1
    fi
    
    if [[ -z "$AGENT_NAME" ]]; then
        log_error "Agent name not specified"
        exit 1
    fi
    
    if [[ ! "$QUALITY_GATE_LEVEL" =~ ^(pre_commit|pre_merge|pre_release)$ ]]; then
        log_error "Invalid quality gate level: $QUALITY_GATE_LEVEL"
        echo "Valid levels: pre_commit, pre_merge, pre_release"
        exit 1
    fi
}

# Load agent-specific configuration
load_agent_config() {
    local config_file="$QUALITY_CONFIG_DIR/agents/${AGENT_NAME}.yml"
    
    if [[ ! -f "$config_file" ]]; then
        log_error "Agent configuration not found: $config_file"
        exit 1
    fi
    
    log_info "Loading configuration for agent: $AGENT_NAME"
    export AGENT_CONFIG_FILE="$config_file"
}

# Execute static analysis
run_static_analysis() {
    log_info "Running static analysis for $AGENT_NAME..."
    
    local analysis_report="$REPORTS_DIR/${AGENT_NAME}_static_analysis_$(date +%Y%m%d_%H%M%S).json"
    
    # Run clang-static-analyzer
    if command -v clang-static-analyzer >/dev/null 2>&1; then
        log_info "Running clang-static-analyzer..."
        find "$COMPONENT_PATH" -name "*.c" -o -name "*.cpp" | \
            xargs clang-static-analyzer --format=json > "$analysis_report.clang" 2>/dev/null || true
    fi
    
    # Run cppcheck
    if command -v cppcheck >/dev/null 2>&1; then
        log_info "Running cppcheck..."
        cppcheck --enable=all --xml --xml-version=2 "$COMPONENT_PATH" 2> "$analysis_report.cppcheck" || true
    fi
    
    # Run custom security analysis
    "$SCRIPT_DIR/security_analyzer.py" --agent "$AGENT_NAME" --path "$COMPONENT_PATH" --output "$analysis_report.security"
    
    # Aggregate results
    "$SCRIPT_DIR/aggregate_analysis.py" \
        --clang "$analysis_report.clang" \
        --cppcheck "$analysis_report.cppcheck" \
        --security "$analysis_report.security" \
        --output "$analysis_report" \
        --agent "$AGENT_NAME"
    
    # Check results against thresholds
    local critical_issues=$(jq '.summary.critical_issues' "$analysis_report")
    local high_issues=$(jq '.summary.high_issues' "$analysis_report")
    
    if [[ "$critical_issues" -gt 0 ]]; then
        log_error "Static analysis failed: $critical_issues critical issues found"
        jq '.critical_issues[]' "$analysis_report"
        return 1
    fi
    
    if [[ "$high_issues" -gt 5 ]]; then
        log_error "Static analysis failed: $high_issues high-severity issues found (limit: 5)"
        jq '.high_issues[]' "$analysis_report"
        return 1
    fi
    
    log_success "Static analysis passed"
    return 0
}

# Execute unit tests
run_unit_tests() {
    log_info "Running unit tests for $AGENT_NAME..."
    
    local test_report="$REPORTS_DIR/${AGENT_NAME}_unit_tests_$(date +%Y%m%d_%H%M%S).xml"
    
    # Find and run tests based on agent type
    case "$AGENT_NAME" in
        "kernel-architect"|"memory-manager"|"hardware-compat-expert")
            # Kernel-level tests
            if [[ -f "$COMPONENT_PATH/tests/Makefile" ]]; then
                cd "$COMPONENT_PATH/tests"
                make clean && make test
                ./test_runner --xml-output="$test_report"
            fi
            ;;
        "ux-wizard"|"multitasking-maestro"|"brand-identity-guru")
            # UI tests - may require virtual display
            export DISPLAY=:99
            Xvfb :99 -screen 0 1024x768x24 &
            local xvfb_pid=$!
            
            if [[ -f "$COMPONENT_PATH/tests/run_ui_tests.sh" ]]; then
                "$COMPONENT_PATH/tests/run_ui_tests.sh" --report="$test_report"
            fi
            
            kill $xvfb_pid 2>/dev/null || true
            ;;
        *)
            # Generic test runner
            if [[ -f "$COMPONENT_PATH/tests/test_runner" ]]; then
                "$COMPONENT_PATH/tests/test_runner" --xml-output="$test_report"
            elif [[ -f "$COMPONENT_PATH/run_tests.sh" ]]; then
                "$COMPONENT_PATH/run_tests.sh" --xml-output="$test_report"
            fi
            ;;
    esac
    
    # Parse test results
    if [[ -f "$test_report" ]]; then
        local failed_tests=$(xmllint --xpath "count(//testcase[@status='failed'])" "$test_report" 2>/dev/null || echo "0")
        local total_tests=$(xmllint --xpath "count(//testcase)" "$test_report" 2>/dev/null || echo "0")
        
        if [[ "$failed_tests" -gt 0 ]]; then
            log_error "Unit tests failed: $failed_tests out of $total_tests tests failed"
            xmllint --xpath "//testcase[@status='failed']" "$test_report" 2>/dev/null || true
            return 1
        fi
        
        log_success "Unit tests passed: $total_tests tests executed successfully"
    else
        log_error "Unit test report not found: $test_report"
        return 1
    fi
    
    return 0
}

# Check code coverage
check_code_coverage() {
    log_info "Checking code coverage for $AGENT_NAME..."
    
    local coverage_threshold=80
    local coverage_report="$REPORTS_DIR/${AGENT_NAME}_coverage_$(date +%Y%m%d_%H%M%S).json"
    
    # Agent-specific coverage thresholds
    case "$AGENT_NAME" in
        "kernel-architect"|"privacy-security-engineer"|"memory-manager")
            coverage_threshold=95  # Critical components need higher coverage
            ;;
        "gaming-layer-engineer"|"ux-wizard"|"multitasking-maestro")
            coverage_threshold=85  # Performance-critical components
            ;;
        *)
            coverage_threshold=80  # Standard components
            ;;
    esac
    
    # Run coverage analysis
    if command -v gcov >/dev/null 2>&1; then
        # Generate coverage data
        find "$COMPONENT_PATH" -name "*.gcda" -exec gcov {} \; >/dev/null 2>&1 || true
        
        # Use lcov if available
        if command -v lcov >/dev/null 2>&1; then
            lcov --capture --directory "$COMPONENT_PATH" --output-file "$coverage_report.info"
            lcov --summary "$coverage_report.info" | grep "lines" | \
                awk '{print $2}' | sed 's/%//' > "$coverage_report.percent"
        fi
    fi
    
    # Parse coverage results
    if [[ -f "$coverage_report.percent" ]]; then
        local coverage_percent=$(cat "$coverage_report.percent" | head -1)
        
        if (( $(echo "$coverage_percent < $coverage_threshold" | bc -l) )); then
            log_error "Code coverage below threshold: ${coverage_percent}% < ${coverage_threshold}%"
            return 1
        fi
        
        log_success "Code coverage check passed: ${coverage_percent}% >= ${coverage_threshold}%"
    else
        log_error "Could not determine code coverage"
        return 1
    fi
    
    return 0
}

# Run security scan
run_security_scan() {
    log_info "Running security scan for $AGENT_NAME..."
    
    local security_report="$REPORTS_DIR/${AGENT_NAME}_security_$(date +%Y%m%d_%H%M%S).json"
    
    # Run multiple security scanners
    "$SCRIPT_DIR/security_scanner.py" \
        --agent "$AGENT_NAME" \
        --path "$COMPONENT_PATH" \
        --output "$security_report" \
        --level "$QUALITY_GATE_LEVEL"
    
    # Check results
    local critical_vulns=$(jq '.summary.critical_vulnerabilities' "$security_report")
    local high_vulns=$(jq '.summary.high_vulnerabilities' "$security_report")
    
    if [[ "$critical_vulns" -gt 0 ]]; then
        log_error "Security scan failed: $critical_vulns critical vulnerabilities found"
        jq '.critical_vulnerabilities[]' "$security_report"
        return 1
    fi
    
    # For pre-commit, allow some high-severity issues
    local high_vuln_limit=0
    if [[ "$QUALITY_GATE_LEVEL" == "pre_commit" ]]; then
        high_vuln_limit=2
    fi
    
    if [[ "$high_vulns" -gt "$high_vuln_limit" ]]; then
        log_error "Security scan failed: $high_vulns high-severity vulnerabilities found (limit: $high_vuln_limit)"
        jq '.high_vulnerabilities[]' "$security_report"
        return 1
    fi
    
    log_success "Security scan passed"
    return 0
}

# Run integration tests
run_integration_tests() {
    log_info "Running integration tests for $AGENT_NAME..."
    
    local integration_report="$REPORTS_DIR/${AGENT_NAME}_integration_$(date +%Y%m%d_%H%M%S).xml"
    
    # Find related agents for integration testing
    local related_agents=$(yq eval ".agents.${AGENT_NAME}.integration_dependencies[]" "$QUALITY_CONFIG_DIR/integration_matrix.yml" 2>/dev/null || echo "")
    
    if [[ -n "$related_agents" ]]; then
        log_info "Running integration tests with related agents: $related_agents"
        
        # Start integration test framework
        "$SCRIPT_DIR/integration_test_runner.py" \
            --primary-agent "$AGENT_NAME" \
            --related-agents "$related_agents" \
            --component-path "$COMPONENT_PATH" \
            --output "$integration_report"
        
        # Parse results
        local failed_integrations=$(xmllint --xpath "count(//testcase[@status='failed'])" "$integration_report" 2>/dev/null || echo "0")
        
        if [[ "$failed_integrations" -gt 0 ]]; then
            log_error "Integration tests failed: $failed_integrations integration points failed"
            return 1
        fi
        
        log_success "Integration tests passed"
    else
        log_info "No integration tests required for $AGENT_NAME"
    fi
    
    return 0
}

# Run performance tests
run_performance_tests() {
    log_info "Running performance tests for $AGENT_NAME..."
    
    local perf_report="$REPORTS_DIR/${AGENT_NAME}_performance_$(date +%Y%m%d_%H%M%S).json"
    
    # Agent-specific performance tests
    case "$AGENT_NAME" in
        "kernel-architect")
            "$SCRIPT_DIR/perf_tests/kernel_performance.py" --output "$perf_report"
            ;;
        "gaming-layer-engineer")
            "$SCRIPT_DIR/perf_tests/graphics_performance.py" --output "$perf_report"
            ;;
        "audio-subsystem-engineer")
            "$SCRIPT_DIR/perf_tests/audio_performance.py" --output "$perf_report"
            ;;
        "network-architect")
            "$SCRIPT_DIR/perf_tests/network_performance.py" --output "$perf_report"
            ;;
        *)
            "$SCRIPT_DIR/perf_tests/generic_performance.py" --agent "$AGENT_NAME" --path "$COMPONENT_PATH" --output "$perf_report"
            ;;
    esac
    
    # Check performance regression
    "$SCRIPT_DIR/performance_regression_detector.py" \
        --current "$perf_report" \
        --agent "$AGENT_NAME" \
        --threshold 0.05
    
    local regression_status=$?
    
    if [[ $regression_status -ne 0 ]]; then
        log_error "Performance regression detected"
        return 1
    fi
    
    log_success "Performance tests passed"
    return 0
}

# Generate quality gate report
generate_report() {
    local gate_level=$1
    local status=$2
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    
    local report_file="$REPORTS_DIR/${AGENT_NAME}_${gate_level}_$(date +%Y%m%d_%H%M%S).json"
    
    cat > "$report_file" <<EOF
{
    "quality_gate": {
        "level": "$gate_level",
        "agent": "$AGENT_NAME",
        "component_path": "$COMPONENT_PATH",
        "timestamp": "$timestamp",
        "status": "$status",
        "checks": {
            "static_analysis": $([ -f "$REPORTS_DIR/${AGENT_NAME}_static_analysis_"*.json ] && echo "true" || echo "false"),
            "unit_tests": $([ -f "$REPORTS_DIR/${AGENT_NAME}_unit_tests_"*.xml ] && echo "true" || echo "false"),
            "code_coverage": $([ -f "$REPORTS_DIR/${AGENT_NAME}_coverage_"*.percent ] && echo "true" || echo "false"),
            "security_scan": $([ -f "$REPORTS_DIR/${AGENT_NAME}_security_"*.json ] && echo "true" || echo "false"),
            "integration_tests": $([ -f "$REPORTS_DIR/${AGENT_NAME}_integration_"*.xml ] && echo "true" || echo "false"),
            "performance_tests": $([ -f "$REPORTS_DIR/${AGENT_NAME}_performance_"*.json ] && echo "true" || echo "false")
        }
    }
}
EOF
    
    log_info "Quality gate report generated: $report_file"
}

# Main execution function
execute_quality_gate() {
    local gate_level=$1
    
    log_info "Executing Quality Gate Level: $gate_level for Agent: $AGENT_NAME"
    
    case $gate_level in
        "pre_commit")
            log_info "Running Pre-Commit Quality Checks..."
            
            run_static_analysis || { generate_report "$gate_level" "FAILED"; exit 1; }
            run_unit_tests || { generate_report "$gate_level" "FAILED"; exit 1; }
            check_code_coverage || { generate_report "$gate_level" "FAILED"; exit 1; }
            run_security_scan || { generate_report "$gate_level" "FAILED"; exit 1; }
            
            log_success "Pre-Commit Quality Gate: PASSED"
            generate_report "$gate_level" "PASSED"
            ;;
            
        "pre_merge")
            log_info "Running Pre-Merge Quality Checks..."
            
            # Run all pre-commit checks first
            run_static_analysis || { generate_report "$gate_level" "FAILED"; exit 1; }
            run_unit_tests || { generate_report "$gate_level" "FAILED"; exit 1; }
            check_code_coverage || { generate_report "$gate_level" "FAILED"; exit 1; }
            run_security_scan || { generate_report "$gate_level" "FAILED"; exit 1; }
            
            # Additional pre-merge checks
            run_integration_tests || { generate_report "$gate_level" "FAILED"; exit 1; }
            run_performance_tests || { generate_report "$gate_level" "FAILED"; exit 1; }
            
            log_success "Pre-Merge Quality Gate: PASSED"
            generate_report "$gate_level" "PASSED"
            ;;
            
        "pre_release")
            log_info "Running Pre-Release Quality Checks..."
            
            # Run comprehensive quality validation
            "$SCRIPT_DIR/system_tests/run_system_tests.py" --agent "$AGENT_NAME"
            "$SCRIPT_DIR/compatibility_tests/run_compatibility_tests.py" --agent "$AGENT_NAME"
            "$SCRIPT_DIR/security_tests/run_penetration_tests.py" --agent "$AGENT_NAME"
            "$SCRIPT_DIR/performance_tests/run_benchmarks.py" --agent "$AGENT_NAME"
            
            log_success "Pre-Release Quality Gate: PASSED"
            generate_report "$gate_level" "PASSED"
            ;;
            
        *)
            log_error "Invalid quality gate level: $gate_level"
            exit 1
            ;;
    esac
}

# Main script execution
main() {
    validate_inputs
    load_agent_config
    
    # Set component path if not provided
    if [[ -z "$COMPONENT_PATH" ]]; then
        case "$AGENT_NAME" in
            "kernel-architect")
                COMPONENT_PATH="$PROJECT_ROOT/kernel"
                ;;
            "driver-integration-specialist")
                COMPONENT_PATH="$PROJECT_ROOT/drivers"
                ;;
            "ux-wizard"|"multitasking-maestro")
                COMPONENT_PATH="$PROJECT_ROOT/kernel/ui"
                ;;
            "raeen-studio-lead")
                COMPONENT_PATH="$PROJECT_ROOT/userland/raeenstudio"
                ;;
            *)
                COMPONENT_PATH="$PROJECT_ROOT"
                ;;
        esac
    fi
    
    log_info "Starting quality gate validation for $AGENT_NAME (Level: $QUALITY_GATE_LEVEL)"
    log_info "Component path: $COMPONENT_PATH"
    
    execute_quality_gate "$QUALITY_GATE_LEVEL"
    
    log_success "Quality gate validation completed successfully"
}

# Execute main function
main "$@"