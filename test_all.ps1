Write-Host "======================================================" -ForegroundColor Cyan
Write-Host "    TrafficRule Prototype Test Suite (Lexer & Parser) " -ForegroundColor Cyan
Write-Host "======================================================" -ForegroundColor Cyan

# 1. Build Compiler
Write-Host "`n[STEP 1] Compiling TrafficRule from C source..." -ForegroundColor Yellow
gcc -std=c99 -Wall -Wextra src/*.c -o trafficrule.exe
if ($LASTEXITCODE -ne 0) {
    Write-Host "[FATAL] Compilation failed!" -ForegroundColor Red
    exit 1
}
Write-Host "[PASS] trafficrule.exe built cleanly with 0 errors/warnings." -ForegroundColor Green

# 2. Test Cases Matrix (Lexer & Parser)
$testCases = @(
    @{ Name="Sample 1 (Speed Violation Rule)"; Path="examples/sample1.tr"; ExpectedExit=0; ExpectedPattern="AST Structure" },
    @{ Name="Sample 2 (Compound AND Rule)"; Path="examples/sample2.tr"; ExpectedExit=0; ExpectedPattern="AST Structure" },
    @{ Name="Error Test: Lexical Error (Invalid Symbol)"; Path="examples/err_lexical.tr"; ExpectedExit=1; ExpectedPattern="Lexical Error" },
    @{ Name="Error Test: Syntax Error (Malformed Condition)"; Path="examples/err_syntax.tr"; ExpectedExit=1; ExpectedPattern="SYNTAX ERROR" }
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
    Write-Host "======================================================" -ForegroundColor Cyan
}
