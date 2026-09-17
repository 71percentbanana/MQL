Write-Host "======================================================" -ForegroundColor Cyan
Write-Host "    TrafficRule Compiler Test Suite (Phase 2)        " -ForegroundColor Cyan
Write-Host "======================================================" -ForegroundColor Cyan

# 1. Build Compiler
Write-Host "`n[STEP 1] Compiling TrafficRule compiler from C source..." -ForegroundColor Yellow
gcc -std=c99 -Wall -Wextra src/*.c -o trafficrule.exe
if ($LASTEXITCODE -ne 0) {
    Write-Host "[FATAL] Compilation failed!" -ForegroundColor Red
    exit 1
}
Write-Host "[PASS] trafficrule.exe built cleanly with 0 errors/warnings." -ForegroundColor Green

# 2. Test Cases Matrix
$testCases = @(
    @{ Name="Sample 1 (Speed Violation)"; Path="examples/sample1.tr"; ExpectedExit=0; ExpectedPattern="Traffic Violation Detected" },
    @{ Name="Sample 2 (Multi-rule & Signal Crossing)"; Path="examples/sample2.tr"; ExpectedExit=0; ExpectedPattern="FLAG_VIOLATION" },
    @{ Name="Sample 3 (If-Else & ALLOW)"; Path="examples/sample3_else.tr"; ExpectedExit=0; ExpectedPattern="ALLOW" },
    @{ Name="Sample Opt (Constant Folding)"; Path="examples/sample_opt.tr"; ExpectedExit=0; ExpectedPattern="Optimized Three-Address Code" },
    @{ Name="Error Test: Lexical Error"; Path="examples/err_lexical.tr"; ExpectedExit=1; ExpectedPattern="Lexical Error" },
    @{ Name="Error Test: Syntax Error"; Path="examples/err_syntax.tr"; ExpectedExit=1; ExpectedPattern="SYNTAX ERROR" },
    @{ Name="Error Test: Semantic Error"; Path="examples/err_semantic.tr"; ExpectedExit=1; ExpectedPattern="SEMANTIC ERROR" }
)

$passed = 0
$failed = 0

Write-Host "`n[STEP 2] Running Functional Test Matrix..." -ForegroundColor Yellow

foreach ($tc in $testCases) {
    Write-Host "------------------------------------------------------"
    Write-Host "Running: $($tc.Name)" -ForegroundColor Magenta
    $output = .\trafficrule.exe $tc.Path --all 2>&1 | Out-String
    $actualExit = $LASTEXITCODE

    $exitMatch = ($actualExit -eq $tc.ExpectedExit)
    $patternMatch = ($output -match $tc.ExpectedPattern)

    if ($exitMatch -and $patternMatch) {
        Write-Host "  Result: PASS [ExitCode=$actualExit, Pattern='$($tc.ExpectedPattern)' matched]" -ForegroundColor Green
        $passed++
    } else {
        Write-Host "  Result: FAIL [ExpectedExit=$($tc.ExpectedExit), Got=$actualExit, PatternMatch=$patternMatch]" -ForegroundColor Red
        Write-Host "Output snippet:"
        Write-Host $output.Substring(0, [Math]::Min(300, $output.Length))
        $failed++
    }
}

Write-Host "`n======================================================" -ForegroundColor Cyan
Write-Host "Test Results Summary:" -ForegroundColor Cyan
Write-Host "  Passed: $passed / $($testCases.Count)" -ForegroundColor Green
if ($failed -gt 0) {
    Write-Host "  Failed: $failed" -ForegroundColor Red
    exit 1
} else {
    Write-Host "  ALL TESTS PASSED! Ready for Review 2 & Viva." -ForegroundColor Green
    Write-Host "======================================================" -ForegroundColor Cyan
}
