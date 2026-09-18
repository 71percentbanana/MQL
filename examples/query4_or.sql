-- Query 4: Disjunctive filter with logical OR
SELECT name, city 
FROM students 
WHERE city = 'Seattle' OR city = 'Boston' 
ORDER BY name ASC;
