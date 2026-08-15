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

Teachers start an attendance session for an assigned subject.
The system generates a **4-character OTP** that is valid for exactly **120 seconds**.

Teacher
   ↓
Select Subject
   ↓
Start Attendance Session
   ↓
4-Character OTP Generated
   ↓
Valid for 120 Seconds
   ↓
Student Submits OTP
   ↓
System Validates Session + OTP + Expiry
   ↓
Attendance Recorded

## Attendance Formula

 (Attended Sessions + Medical Exemptions)
---------------------------------------- × 100
           Total Sessions Held

## Default Admin

Username: admin
Password: admin123

## Demo Accounts

Teachers:
drsingh / teacher123
profkaur / teacher123
profverma / teacher123

Students:
stu001 / student123
stu002 / student123
stu003 / student123

## Demo Subjects

CS101 - Data Structures & Algorithms
CS102 - Operating Systems
CS201 - Database Management Systems
CS301 - Computer Networks
