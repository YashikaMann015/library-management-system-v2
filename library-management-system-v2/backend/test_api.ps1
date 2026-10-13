Write-Host "Testing Library C API..." -ForegroundColor Cyan

Write-Host "`n1. Health"
Invoke-RestMethod http://localhost:8080/health | ConvertTo-Json

Write-Host "`n2. Login"
$login = Invoke-RestMethod http://localhost:8080/api/auth/login -Method Post -ContentType "application/json" -Body '{"email":"student@library.local","password":"student123"}'
$login | ConvertTo-Json
$token = $login.token

$headers = @{ Authorization = "Bearer $token" }

Write-Host "`n3. Me"
Invoke-RestMethod http://localhost:8080/api/auth/me -Headers $headers | ConvertTo-Json

Write-Host "`n4. Books"
Invoke-RestMethod http://localhost:8080/api/books -Headers $headers | ConvertTo-Json
