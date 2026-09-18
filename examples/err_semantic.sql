-- Semantic Error Test Case: Referencing unknown column and comparing type mismatch
SELECT name, nonexistent_column 
FROM students 
WHERE age = 'twenty';
