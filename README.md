# Attendance Management System

A desktop-based **Attendance Management System** built with **C++17** and **Qt Widgets**.
The application provides separate portals for **Admin, Teacher, and Student** with role-based access, time-limited OTP attendance, medical attendance exemptions, attendance eligibility, admit-card processing, audit logging, and local data persistence.

## Features

- Role-Based Access Control (RBAC)
- Admin, Teacher, and Student portals
- Admin-only Teacher and Student account creation
- 4-character attendance OTP
- 120-second OTP validity
- Duplicate attendance protection
- Medical attendance exemptions
- Attendance percentage calculation
- 75% admit-card eligibility rule
- Admit-card generation and seat allocation
- SHA-256 admit-card integrity verification
- Audit logging
- Local JSON persistence
- Dark-themed desktop UI
- Demo users, subjects, and attendance data

## Attendance System

Teachers start an attendance session for an assigned subject.

```text
Teacher
  |
  v
Select Subject
  |
  v
Start Attendance Session
  |
  v
4-Character OTP Generated
  |
  v
OTP Valid for 120 Seconds
  |
  v
Student Submits OTP
  |
  v
System Validates OTP, Subject, Session and Expiry
  |
  v
Attendance Recorded
```

### Attendance Rules

| Rule | Value |
|---|---|
| OTP Length | 4 characters |
| OTP Validity | 120 seconds |
| Duplicate Submission | Rejected |
| Expired OTP | Rejected |
| Admit Card Threshold | 75% |

### Attendance Formula

The system calculates attendance using attended sessions and approved medical exemptions.

```text
Attendance % = (( Attended Sessions + Medical Exemptions ) / Total Sessions Held ) × 100
```

### Example

```text
Total Sessions Held = 20
Attended Sessions   = 13
Medical Exemptions  = 2

Attendance % = ((13 + 2) / 20) × 100
             = 75%
```

### Eligibility

```text
Attendance >= 75%  ->  Admit Card Eligible
Attendance < 75%   ->  Admit Card Withheld
```

## Medical Attendance

Medical attendance is an authorized exemption granted by an Admin or an authorized Teacher.

```text
Total Sessions Held = 20
Attended Sessions   = 12
Medical Exemptions  = 3

Attendance % = ((12 + 3) / 20) × 100
             = 75%
```

Medical exemptions:

- Are stored separately from normal attendance
- Are included in the eligibility calculation
- Cannot be added by Students
- Are validated by the core system
- Are recorded in the audit log

## Admit Card System

```text
Attendance >= 75%
        |
        v
     APPROVED
        |
        +--> Examination Seat Assigned
        +--> Admit Card Generated
        +--> SHA-256 Signature Created
```

```text
Attendance < 75%
        |
        v
     WITHHELD
        |
        +--> No Examination Seat
        +--> Admit Card Withheld
```

## SHA-256 Verification

Important admit-card information is converted into a verification payload and hashed using SHA-256.

```text
Admit Card Data
      |
      v
   SHA-256
      |
      v
Stored Signature
      |
      v
  Verification
      |
      +----> VERIFIED
      |
      +----> INVALID
```

If important admit-card data is changed after generation, the calculated signature no longer matches the stored signature.

## Roles

### Admin

- Create Teacher accounts
- Create Student accounts
- Manage users
- Manage subjects/courses
- Generate admit cards
- Review attendance
- View audit logs

**Only Admin can create Teacher and Student accounts.**

### Teacher

- View assigned subjects
- Start attendance sessions
- Generate attendance OTP
- View student attendance
- Grant permitted medical exemptions

Teachers cannot create user accounts or access Admin-only controls.

### Student

- Submit attendance OTP
- View attendance percentage
- View attendance history
- View medical exemptions
- View admit-card status

Students cannot modify attendance or create accounts.

## Encapsulation

Protected application state is kept inside the core controller and accessed through controlled operations.

This prevents Students and Teachers from directly modifying:

- User accounts
- Attendance records
- Medical exemptions
- Attendance sessions
- Admit-card decisions

## Default Administrator

```text
Username: admin
Password: admin123
Role: Administrator
```

## Demo Teachers

| Username | Password | Name |
|---|---|---|
| `drsingh` | `teacher123` | Dr. Rajesh Singh |
| `profkaur` | `teacher123` | Prof. Amrita Kaur |
| `profverma` | `teacher123` | Prof. Rohan Verma |

## Demo Students

| Username | Password | Name |
|---|---|---|
| `stu001` | `student123` | Arjun Sharma |
| `stu002` | `student123` | Priya Verma |
| `stu003` | `student123` | Karan Mehta |

## Demo Subjects

| Code | Subject | Teacher |
|---|---|---|
| `CS101` | Data Structures & Algorithms | Dr. Rajesh Singh |
| `CS102` | Operating Systems | Dr. Rajesh Singh |
| `CS201` | Database Management Systems | Prof. Amrita Kaur |
| `CS301` | Computer Networks | Prof. Rohan Verma |

Demo attendance and medical-exemption records are included so the attendance, eligibility, and admit-card workflows can be demonstrated immediately.

## Technology Stack

- **Language:** C++17
- **Framework:** Qt Widgets
- **IDE:** Qt Creator
- **Build System:** CMake
- **Hashing:** SHA-256
- **Storage:** Local JSON

## Project Structure

```text
Attendance-Management-System/
├── main.cpp
├── README.md
├── .gitignore
├── CMakeLists.txt
└── .github/
    └── workflows/
        └── build.yml
```

## Build

### Requirements

- Qt 6.x
- Qt Widgets
- C++17-compatible compiler
- Qt Creator
- CMake

Open the project in Qt Creator, configure a Desktop Qt Kit, configure CMake, build, and run.

## Academic Project

This project is developed as a **B.Tech academic project** demonstrating:

- C++
- Object-Oriented Programming
- Encapsulation
- Role-Based Access Control
- Authentication and Authorization
- Qt GUI development
- Attendance automation
- Data persistence
- Audit logging
- Cryptographic integrity verification

> **Note:** The default and demo credentials are provided for development and demonstration purposes. They should be changed or removed before any real deployment.
