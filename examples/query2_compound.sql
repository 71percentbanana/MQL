-- Query 2: Multi-condition filtering with logical AND
SELECT name, department, salary 
FROM employees 
WHERE salary >= 70000 AND department = 'Engineering' 
ORDER BY salary DESC;
