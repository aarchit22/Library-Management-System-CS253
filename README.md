# Library Management System CS253

## Overview

The Library Management System is a C++ console application that manages the operations of a library. It supports multiple user roles (Students, Faculty, and Librarians) and implements features such as book borrowing, book returning, reservations, fine tracking, and fine clearance requests. All data is persisted in CSV files, ensuring that the state of the library is maintained between program runs.

## Features

- **User Roles:**
  - **Students:**  
    - Can borrow up to 3 books at a time.
    - Overdue books (beyond 15 days) incur a fine of ₹10 per day.
    - Can reserve a book if it is currently borrowed.
  - **Faculty:**  
    - Can borrow up to 5 books at a time.
    - No overdue fines.
    - Can reserve a book if it is currently borrowed.
  - **Librarians:**  
    - Manage the library operations.
    - Can add and remove books and users.
    - Can search for books and view overall issued records.
    - Approve fine clearance requests.
  
- **Reservation System:**
  - If a book is borrowed by another user, a user can reserve it for 5 days following the day of return of book.
  - Within this 5-day period, only the reserving user will see the book as available. If not borrowed within this period, the reservation expires and the book becomes available to all.

- **Borrowing Constraints:**
  - Students cannot borrow more than 3 books at a time.
  - Faculty cannot borrow more than 5 books at a time.
  - Any user with an outstanding fine or a pending fine clearance request will be prevented from borrowing new books.

- **Fine Management:**
  - Students who return books after the allowed borrowing period incur a fine.
  - Users can request fine clearance, which a librarian must approve before the fine is reset.

- **Data Persistence:**
  - All books, users, and issued record information is stored in CSV files:
    - `books.csv`
    - `users.csv`
    - `issued.csv`
  - Data is loaded at program startup and updated after every operation.

- **Input Validation:**
  - User input is validated (using `cin.clear()` and `cin.ignore()`) to prevent infinite loops from non-numeric or invalid choices.

## Installation

1. **Requirements:**
   - A C++ compiler (e.g., g++ with C++11 support or later).
   - A terminal or command prompt.

2. **Steps:**
   - Copy the complete source code (provided in LibraryManagementSystem.cpp) into your project directory.
   - Open a terminal in the project directory.
   - Compile the program, for example:
     ```
     g++ -std=c++11 -o LibraryManagementSystem LibraryManagementSystem.cpp
     ```
   - Run the program:
     ```
     ./LibraryManagementSystem    (Linux/Mac)
     LibraryManagementSystem.exe  (Windows)
     ```

## Usage

- **Login:**
  - At startup, the application loads data from CSV files. If none exist, default sample data is preloaded.
  - Users can log in using their unique user ID and password.
  
- **Student/Faculty Options:**
  - **View All Books:** Displays list of all books in the library.
  - **Borrow Book:** Issues a book if the user is within the borrowing limit and has no outstanding fines.
  - **Reserve Book:** If the book is currently borrowed by someone else, a user can reserve it.
  - **Return Book:** Returns a borrowed book; if overdue (beyond allowed days) a fine is applied.
  - **View Outstanding Fine:** Displays the current fine amount.
  - **Request Fine Clearance:** Users can request that librarians clear their outstanding fines.
  - **View Currently Borrowed Books:** Displays all books that the user currently has issued.
  
- **Librarian Options:**
  - **Add Book:** Add new books to the library (duplicate ISBNs are prevented).
  - **Add User:** Add new users to the library (duplicate user IDs are prevented).
  - **Remove User:** Remove a user (only if they have no actively borrowed books).
  - **Search Book by ISBN:** Find a book by ISBN.
  - **List All Books:** Display all books.
  - **View Borrowing Details:** View overall list of currently issued books.
  - **List All Users:** Display list of all user accounts.
  - **Approve Fine Clearance:** Approve outstanding fine clearance requests.

## File Structure

- **LibraryManagementSystem.cpp:**  
  Contains the complete source code including classes for Book, Account, and User (with derived classes for Student, Faculty, and Librarian), as well as file I/O functionality and the main menu handling.

- **books.csv:**  
  Stores information about each book (title, author, publisher, year, ISBN, status, reservedBy, reservationExpiry).

- **users.csv:**  
  Stores user account information (user id, name, password, role, outstanding fine).

- **issued.csv:**  
  Stores issued book records (user id, ISBN, issue timestamp).

## Notes

- The application enforces borrowing limits strictly. If a user (student or faculty) has reached their maximum limit of issued books, further borrow requests will be rejected.
- Only one reservation per book is allowed. If a book is already reserved by one user, further reservation requests will be denied.
- Books reserved by a user become available exclusively to that user for 5 days after return, after which the reservation expires.
- All changes are persisted in CSV files for long-term storage. Make sure that the application has permissions to read/write to these files.
- Input errors are handled to prevent infinite loops due to invalid input.

## License

This project is provided as-is. You may use, modify, and distribute it as per your requirements.

