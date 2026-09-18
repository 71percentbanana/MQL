# =====================================================================
# SQL2Mongo Test Suite Automation
# Course: BCSE307L Compiler Design Laboratory
# Author: Alan James (24BCT0181)
# =====================================================================

Write-Host "======================================================================" -ForegroundColor Cyan
Write-Host "  Running Automated Test Suite: SQL2Mongo Compiler & Execution Engine " -ForegroundColor Cyan
Write-Host "======================================================================" -ForegroundColor Cyan

$Passed = 0
$Failed = 0

function Run-Test {
    param(
        [string]$Name,
        [string]$File,
        [int]$ExpectedExitCode,
        [string]$Flag = "--all"
    )

    Write-Host "`n>>> RUNNING TEST: $Name [$File] ..." -ForegroundColor Yellow
    $output = .\sql2mongo.exe $File $Flag 2>&1
    $exitCode = $LASTEXITCODE

    if ($exitCode -eq $ExpectedExitCode) {
        Write-Host "[PASS] $Name (Exit Code: $exitCode as expected)" -ForegroundColor Green
        $global:Passed++
    } else {
        Write-Host "[FAIL] $Name (Expected: $ExpectedExitCode, Got: $exitCode)" -ForegroundColor Red
        $global:Failed++
    }
}

# Test Cases
Run-Test "1. Basic SELECT, WHERE, ORDER BY, LIMIT" "examples/query1_basic.sql" 0 "--all"
Run-Test "2. Compound Filter with AND Condition"   "examples/query2_compound.sql" 0 "--all"
Run-Test "3. Wildcard Projection (SELECT *)"       "examples/query3_wildcard.sql" 0 "--all"
Run-Test "4. Disjunctive Filter with OR Condition" "examples/query4_or.sql" 0 "--all"
Run-Test "5. Syntax Error Detection"               "examples/err_syntax.sql" 1
Run-Test "6. Semantic / Schema Error Detection"    "examples/err_semantic.sql" 2

Write-Host "`n======================================================================" -ForegroundColor Cyan
Write-Host "  TEST SUITE RESULTS SUMMARY" -ForegroundColor Cyan
Write-Host "======================================================================" -ForegroundColor Cyan
Write-Host "Total Tests Run : $($Passed + $Failed)"
Write-Host "Tests Passed    : $Passed" -ForegroundColor Green
Write-Host "Tests Failed    : $Failed" -ForegroundColor $(if ($Failed -eq 0) { "Green" } else { "Red" })

if ($Failed -eq 0) {
    Write-Host "`n>>> ALL TESTS PASSED SUCCESSFULLY! <<<" -ForegroundColor Green
} else {
    Write-Host "`n>>> SOME TESTS FAILED <<<" -ForegroundColor Red
}
