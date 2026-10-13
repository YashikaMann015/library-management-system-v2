# Architecture

```text
React Frontend
     |
     | HTTP + JSON
     v
C REST API
     |
     +-- Authentication
     +-- RBAC
     +-- Book service
     +-- Request service
     +-- Issue/return service
     +-- Fine service
     |
     v
SQLite
```

## RBAC

ADMIN has full access.

LIBRARIAN manages operational library workflows.

STUDENT can browse and request books and see their own records.

The backend must enforce role permissions even when frontend routes are hidden.
