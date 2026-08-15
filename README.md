# Attendance Management System

A desktop-based Attendance Management System built with **C++17 and Qt Widgets**.

## Features

- Role-Based Access Control (Admin, Teacher, Student)
- Admin-only Teacher and Student account creation
- 4-character attendance OTP
- 120-second OTP validity
- Duplicate attendance protection
- Medical attendance exemptions
- Attendance percentage calculation
- 75% admit-card eligibility rule
- Admit-card generation and seat allocation
- SHA-256 integrity verification
- Audit logging
- Local JSON persistence
- Dark-themed Qt interface
- Demo Teachers, Students, Subjects, and Attendance data

## Roles

### Admin
- Create Teacher accounts
- Create Student accounts
- Manage users and courses
- Generate admit cards
- View audit logs

### Teacher
- View assigned courses
- Start attendance sessions
- Generate OTP
- View attendance
- Grant permitted medical exemptions

### Student
- Submit attendance OTP
- View attendance
- View medical exemptions
- View admit-card status

## Attendance System

### How Attendance Works

- A Teacher selects an assigned subject.
- The Teacher starts an attendance session.
- The system generates a 4-character OTP.
- The OTP remains valid for 120 seconds.
- Students submit the OTP during the active session.
- The system validates the subject, session, OTP, expiry time, and duplicate submission.
- Valid submissions are recorded as attendance.

### Attendance Rules

- OTP length: 4 characters
- OTP validity: 120 seconds
- Duplicate submissions: Rejected
- Expired OTP submissions: Rejected
- Attendance is calculated from the stored session records.
- Medical exemptions are kept separately and included in the eligibility calculation.

### Attendance Formula

Attendance % = ((Attended Sessions + Medical Exemptions)
                / Total Sessions Held) × 100

## Default Admin

- Username: admin
- Password: admin123

## Demo Accounts

### Teachers

- drsingh / teacher123
- profkaur / teacher123
- profverma / teacher123

### Students

- stu001 / student123
- stu002 / student123
- stu003 / student123

## Demo Subjects

- CS101 - Data Structures & Algorithms
- CS102 - Operating Systems
- CS201 - Database Management Systems
- CS301 - Computer Networks
