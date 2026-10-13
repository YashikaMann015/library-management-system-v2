INSERT OR IGNORE INTO categories(id,name,description) VALUES
(1,'Programming','Programming books'),
(2,'Database','Database systems'),
(3,'Algorithms','Algorithms and data structures'),
(4,'Operating Systems','Operating systems');

INSERT OR IGNORE INTO users(id,name,email,password_hash,role) VALUES
(1,'Rahul Sharma','student@library.local','student123','STUDENT'),
(2,'Priya Librarian','librarian@library.local','librarian123','LIBRARIAN'),
(3,'System Admin','admin@library.local','admin123','ADMIN');

INSERT OR IGNORE INTO books(id,isbn,title,author,publisher,category_id,publication_year,quantity,available_copies,shelf) VALUES
(1,'9780073523323','Database System Concepts','Korth','McGraw Hill',2,2022,5,4,'DB-A1'),
(2,'9780133594140','Operating System Concepts','Silberschatz','Wiley',4,2021,4,2,'OS-B2'),
(3,'9780131103627','The C Programming Language','Kernighan & Ritchie','Prentice Hall',1,2019,5,5,'C-C1'),
(4,'9780262046305','Introduction to Algorithms','Cormen','MIT Press',3,2022,3,1,'ALG-D4');
