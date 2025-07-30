# Bug Report: [Brief Description of the Issue]

**Bug ID:** [AUTO-GENERATED]  
**Date Reported:** [YYYY-MM-DD]  
**Reporter:** [Reporter Name/Username]  
**Assigned To:** [Assignee Name or "Unassigned"]  
**Priority:** [Critical | High | Medium | Low]  
**Severity:** [Blocker | Major | Minor | Trivial]  
**Status:** [New | Assigned | In Progress | Resolved | Verified | Closed | Reopened]

## Summary

[Provide a clear, concise summary of the bug in 1-2 sentences]

## Environment Information

### System Information
- **OS Version:** RaeenOS [version] (build [build number])
- **Kernel Version:** [kernel version]
- **Architecture:** [x86 | x86_64 | ARM | ARM64]
- **Hardware:** [brief hardware description]
- **Memory:** [total RAM]
- **Build Type:** [Debug | Release | Profile]

### Software Context
- **Component/Module:** [affected component]
- **Related Components:** [list related components]
- **Git Commit Hash:** [commit hash if applicable]
- **Build Configuration:** [specific build flags or configuration]

### Development Environment (if applicable)
- **Compiler:** [GCC version, Clang version, etc.]
- **Build Tools:** [make, cmake, etc.]
- **IDE/Editor:** [if relevant to the bug]

## Steps to Reproduce

**Prerequisites:**
[List any setup requirements, special configurations, or initial conditions needed]

**Detailed Steps:**
1. [First step - be specific about actions taken]
2. [Second step - include exact commands, clicks, inputs]
3. [Third step - note any specific timing or conditions]
4. [Continue with all steps needed to reproduce]
5. [Final step that triggers the bug]

**Test Data/Input:**
[Provide specific test data, file contents, command line arguments, or inputs used]

```
[Include relevant code, configuration files, or command examples]
```

## Expected Behavior

[Describe what should happen when following the reproduction steps]

**Expected Results:**
- [Specific expected outcome 1]
- [Specific expected outcome 2]
- [Any relevant success criteria]

## Actual Behavior

[Describe what actually happens - the buggy behavior]

**Observed Results:**
- [Specific observed outcome 1]
- [Specific observed outcome 2]
- [How the behavior differs from expected]

## Evidence and Diagnostics

### Screenshots/Visual Evidence
[Attach or link to screenshots, screen recordings, or visual evidence]
- ![Description](screenshot_url_or_path)
- [Video recording link if applicable]

### Log Output
[Include relevant log output, error messages, console output]

```
[Paste relevant log entries, error messages, stack traces, etc.]
```

### System State
[Include relevant system information captured during the bug]

**Process Information:**
```
[Process listings, memory usage, CPU usage if relevant]
```

**System Resources:**
```
[Memory usage, disk space, network status if relevant]
```

### Code Analysis
[If this is a code-level bug, include relevant code snippets]

**Problematic Code Location:**
- **File:** [filename]
- **Function:** [function name]
- **Line Number:** [approximate line number]

```c
// Relevant code snippet showing the issue
[code snippet]
```

## Impact Assessment

### Severity Justification
[Explain why you assigned this severity level]

**Impact on System:**
- [How does this affect system functionality?]
- [Does it cause crashes, data loss, security issues?]
- [Performance impact if any]

**Impact on Users:**
- [How many users are affected?]
- [Is there a workaround available?]
- [Does it block critical functionality?]

**Impact on Development:**
- [Does it block other development work?]
- [Does it affect testing or release schedules?]

### Frequency
- **Reproducibility:** [Always | Often (>75%) | Sometimes (25-75%) | Rarely (<25%) | Unable to Reproduce]
- **Occurrence Rate:** [How often does this happen in normal usage?]

## Additional Context

### Related Issues
- **Similar Bugs:** [Link to related bug reports]
- **Related Features:** [Link to related feature requests]
- **Regression Information:** [If this is a regression, when was it last working?]

### Workarounds
[Describe any temporary workarounds that can be used]

1. **Workaround 1:** [Description and steps]
   - **Limitations:** [What doesn't work with this workaround]
   - **Risk Assessment:** [Any risks with the workaround]

2. **Workaround 2:** [Description and steps]
   - **Limitations:** [What doesn't work with this workaround]

### Investigation Notes

**Root Cause Analysis:**
[If you have insights into the potential cause]

**Debugging Information:**
[Any debugging you've already done]

```
[Debug output, valgrind reports, static analysis results, etc.]
```

**Potential Solutions:**
[If you have ideas about how to fix it]

## Attachments

### Core Dumps
- [Link to core dump files if available]
- [Instructions for accessing dumps]

### Configuration Files
- [Attach relevant configuration files]
- [System configuration that might be relevant]

### Test Cases
- [Automated test cases that reproduce the issue]
- [Unit tests that fail due to this bug]

### Memory Dumps
- [Memory dump files if relevant]
- [Heap analysis if memory-related]

## Testing Information

### Test Coverage
- **Unit Tests:** [Do existing unit tests catch this? Why not?]
- **Integration Tests:** [Integration test coverage for this area]
- **Manual Testing:** [What manual testing was done]

### Regression Testing
- **Last Known Good Version:** [Version where this worked]
- **First Broken Version:** [Version where this first appeared]
- **Bisect Results:** [If git bisect was used to find the breaking commit]

## Priority Justification

### Business Impact
[Explain the business impact that justifies the priority level]

### Technical Impact
[Explain the technical impact and why it needs this priority]

### Timeline Considerations
[Any deadlines or timeline pressures that affect priority]

## Security Considerations

### Security Impact
[Is this a security vulnerability? Describe the security implications]

- **Attack Vector:** [How could this be exploited?]
- **Confidentiality Impact:** [Does it expose sensitive data?]
- **Integrity Impact:** [Does it allow unauthorized modifications?]
- **Availability Impact:** [Does it cause denial of service?]

### CVE Information
[If applicable, CVE number or security advisory information]

## Resolution Tracking

### Proposed Solution
[Technical approach to fixing the bug - filled in by assignee]

### Implementation Plan
[Step-by-step plan for implementing the fix]

1. [Step 1]
2. [Step 2]
3. [Step 3]

### Testing Plan
[How the fix will be tested]

- **Unit Tests:** [New unit tests to add]
- **Integration Tests:** [Integration tests to add/modify]
- **Regression Tests:** [Ensure fix doesn't break other functionality]
- **Performance Tests:** [If performance impact is possible]

### Code Review Requirements
[Special review requirements for this fix]

## Communication

### Stakeholders
[Who needs to be notified about this bug and its resolution?]

- **Users Affected:** [How to communicate with affected users]
- **Development Teams:** [Which teams need to be informed]
- **Management:** [If escalation is needed]

### Documentation Updates
[What documentation needs to be updated?]

- **User Documentation:** [Release notes, user guides]
- **Developer Documentation:** [API docs, architecture docs]
- **Troubleshooting Guides:** [New troubleshooting entries]

## Verification Criteria

### Definition of Done
[Clear criteria for when this bug is considered fixed]

- [ ] Bug reproduction steps no longer produce the error
- [ ] All related test cases pass
- [ ] Code review completed and approved
- [ ] Documentation updated as needed
- [ ] Regression testing completed
- [ ] Performance impact assessed (if applicable)
- [ ] Security review completed (if security-related)

### Acceptance Testing
[Who will verify the fix and how?]

- **Primary Tester:** [Name and role]
- **Testing Environment:** [Where testing will be done]
- **Testing Timeline:** [When verification will happen]

## Comments and Updates

### Reporter Comments
[Space for the reporter to add additional information]

### Developer Comments
[Space for developers to add technical analysis and updates]

### QA Comments
[Space for QA team to add testing results and verification notes]

---

**Document Control:**
- **Template Version:** 1.0
- **Last Updated:** [YYYY-MM-DD]
- **Review Status:** [Draft | Under Review | Approved]

**Workflow:**
1. Reporter fills out all sections above the Resolution Tracking section
2. Triage team reviews and assigns priority/severity
3. Developer analyzes and fills in Resolution Tracking section
4. Implementation and testing occur
5. QA verifies fix using Verification Criteria
6. Bug is closed when all criteria are met

**Labels/Tags:**
[Add relevant labels: bug, critical, memory-leak, performance, security, regression, etc.]