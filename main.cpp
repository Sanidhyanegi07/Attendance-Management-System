#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTimer>
#include <QProgressBar>
#include <QScrollArea>
#include <QFrame>
#include <QSplitter>
#include <QListWidget>
#include <QTextEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDateTime>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QRandomGenerator>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QFont>
#include <QFontDatabase>
#include <QIcon>
#include <QPixmap>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QGroupBox>
#include <QCheckBox>
#include <QScrollBar>
#include <QAbstractItemView>
#include <QToolTip>
#include <QDebug>
#include <cmath>       // for ceil()
#include <algorithm>

// ─────────────────────────────────────────────
//  ENUMS & CONSTANTS
// ─────────────────────────────────────────────
enum class Role { Admin, Teacher, Student, None };
const int    OTP_LIFETIME_SECONDS = 120;
const int    OTP_LENGTH = 4;
const double ATTENDANCE_THRESHOLD = 75.0;
const QString APP_VERSION         = "2.1";

// ─────────────────────────────────────────────
//  DOMAIN MODELS
// ─────────────────────────────────────────────
struct User {
    QString username, fullName, email, passwordHash, createdAt;
    Role role     = Role::None;
    bool isActive = true;

    QJsonObject toJson() const {
        QJsonObject o;
        o["username"]    = username;
        o["fullName"]    = fullName;
        o["email"]       = email;
        o["passwordHash"]= passwordHash;
        o["role"]        = static_cast<int>(role);
        o["isActive"]    = isActive;
        o["createdAt"]   = createdAt;
        return o;
    }
    static User fromJson(const QJsonObject& o) {
        User u;
        u.username     = o["username"].toString();
        u.fullName     = o["fullName"].toString();
        u.email        = o["email"].toString();
        u.passwordHash = o["passwordHash"].toString();
        u.role         = static_cast<Role>(o["role"].toInt());
        u.isActive     = o["isActive"].toBool(true);
        u.createdAt    = o["createdAt"].toString();
        return u;
    }
};

struct Course {
    QString code, name, description, teacherUsername, semester, createdAt;
    bool isActive = true;

    QJsonObject toJson() const {
        QJsonObject o;
        o["code"]            = code;
        o["name"]            = name;
        o["description"]     = description;
        o["teacherUsername"] = teacherUsername;
        o["isActive"]        = isActive;
        o["semester"]        = semester;
        o["createdAt"]       = createdAt;
        return o;
    }
    static Course fromJson(const QJsonObject& o) {
        Course c;
        c.code            = o["code"].toString();
        c.name            = o["name"].toString();
        c.description     = o["description"].toString();
        c.teacherUsername = o["teacherUsername"].toString();
        c.isActive        = o["isActive"].toBool(true);
        c.semester        = o["semester"].toString();
        c.createdAt       = o["createdAt"].toString();
        return c;
    }
};

struct AttendanceRecord {
    QString studentUsername, courseCode;
    int attendedSessions  = 0;
    int totalSessions     = 0;
    int medicalExemptions = 0;
    QStringList attendedSessionIds;

    QJsonObject toJson() const {
        QJsonObject o;
        o["studentUsername"]   = studentUsername;
        o["courseCode"]        = courseCode;
        o["attendedSessions"]  = attendedSessions;
        o["totalSessions"]     = totalSessions;
        o["medicalExemptions"] = medicalExemptions;
        QJsonArray ids;
        for (const auto& id : attendedSessionIds) ids.append(id);
        o["attendedSessionIds"] = ids;
        return o;
    }
    static AttendanceRecord fromJson(const QJsonObject& o) {
        AttendanceRecord r;
        r.studentUsername   = o["studentUsername"].toString();
        r.courseCode        = o["courseCode"].toString();
        r.attendedSessions  = o["attendedSessions"].toInt();
        r.totalSessions     = o["totalSessions"].toInt();
        r.medicalExemptions = o["medicalExemptions"].toInt();
        for (const auto& v : o["attendedSessionIds"].toArray())
            r.attendedSessionIds.append(v.toString());
        return r;
    }
};

struct Session {
    QString sessionId, courseCode, teacherUsername, otp;
    qint64 startTime  = 0;
    qint64 expiryTime = 0;
    bool   isActive   = true;

    QJsonObject toJson() const {
        QJsonObject o;
        o["sessionId"]       = sessionId;
        o["courseCode"]      = courseCode;
        o["teacherUsername"] = teacherUsername;
        o["otp"]             = otp;
        o["startTime"]       = startTime;
        o["expiryTime"]      = expiryTime;
        o["isActive"]        = isActive;
        return o;
    }
    static Session fromJson(const QJsonObject& o) {
        Session s;
        s.sessionId       = o["sessionId"].toString();
        s.courseCode      = o["courseCode"].toString();
        s.teacherUsername = o["teacherUsername"].toString();
        s.otp             = o["otp"].toString();
        s.startTime       = static_cast<qint64>(o["startTime"].toDouble());
        s.expiryTime      = static_cast<qint64>(o["expiryTime"].toDouble());
        s.isActive        = o["isActive"].toBool(true);
        return s;
    }
};

struct AdmitCard {
    QString cardId, studentUsername, studentFullName, courseCode;
    QString seatNumber, examDate, signature, generatedAt;
    bool approved = false;

    QJsonObject toJson() const {
        QJsonObject o;
        o["cardId"]          = cardId;
        o["studentUsername"] = studentUsername;
        o["studentFullName"] = studentFullName;
        o["courseCode"]      = courseCode;
        o["seatNumber"]      = seatNumber;
        o["examDate"]        = examDate;
        o["signature"]       = signature;
        o["generatedAt"]     = generatedAt;
        o["approved"]        = approved;
        return o;
    }
    static AdmitCard fromJson(const QJsonObject& o) {
        AdmitCard c;
        c.cardId          = o["cardId"].toString();
        c.studentUsername = o["studentUsername"].toString();
        c.studentFullName = o["studentFullName"].toString();
        c.courseCode      = o["courseCode"].toString();
        c.seatNumber      = o["seatNumber"].toString();
        c.examDate        = o["examDate"].toString();
        c.signature       = o["signature"].toString();
        c.generatedAt     = o["generatedAt"].toString();
        c.approved        = o["approved"].toBool();
        return c;
    }
};

struct AuditLog {
    qint64  timestamp = 0;
    QString actor, action, details;

    QJsonObject toJson() const {
        QJsonObject o;
        o["timestamp"] = timestamp;
        o["actor"]     = actor;
        o["action"]    = action;
        o["details"]   = details;
        return o;
    }
    static AuditLog fromJson(const QJsonObject& o) {
        AuditLog l;
        l.timestamp = static_cast<qint64>(o["timestamp"].toDouble());
        l.actor     = o["actor"].toString();
        l.action    = o["action"].toString();
        l.details   = o["details"].toString();
        return l;
    }
};

// ─────────────────────────────────────────────
//  STYLE MANAGER
// ─────────────────────────────────────────────
class StyleManager {
public:
    static QString getStylesheet() {
        return R"(
QWidget {
    background-color: #0f1117;
    color: #e2e8f0;
    font-family: 'Segoe UI', Arial, sans-serif;
    font-size: 13px;
}
QLabel { background: transparent; color: #e2e8f0; }
QLabel#titleLabel {
    font-size: 22px; font-weight: 700;
    color: #f8fafc; letter-spacing: 0.5px;
}
QLabel#subtitleLabel { font-size: 13px; color: #94a3b8; }
QLabel#sectionHeader { font-size: 15px; font-weight: 600; color: #f1f5f9; padding: 4px 0; }
QLabel#otpDisplay {
    font-size: 48px; font-weight: 800; color: #34d399;
    letter-spacing: 12px; background: #0d2818;
    border: 2px solid #34d399; border-radius: 12px; padding: 16px 32px;
}
QLabel#otpExpired {
    font-size: 48px; font-weight: 800; color: #f87171;
    letter-spacing: 8px; background: #1f0f0f;
    border: 2px solid #f87171; border-radius: 12px; padding: 16px 32px;
}
QLabel#badge_ok {
    background: #0d2818; color: #34d399;
    border: 1px solid #34d399; border-radius: 4px;
    padding: 2px 10px; font-size: 11px; font-weight: 600;
}
QLabel#badge_warn {
    background: #1f1a0a; color: #fbbf24;
    border: 1px solid #fbbf24; border-radius: 4px;
    padding: 2px 10px; font-size: 11px; font-weight: 600;
}
QLabel#badge_err {
    background: #1f0f0f; color: #f87171;
    border: 1px solid #f87171; border-radius: 4px;
    padding: 2px 10px; font-size: 11px; font-weight: 600;
}
QLineEdit {
    background-color: #1e2130; border: 1px solid #334155;
    border-radius: 8px; padding: 10px 14px;
    color: #f1f5f9; font-size: 13px;
    selection-background-color: #2563eb;
}
QLineEdit:focus { border: 1.5px solid #3b82f6; background-color: #1e2540; }
QLineEdit:hover { border: 1px solid #475569; }
QLineEdit#otpInput {
    font-size: 22px; font-weight: 700;
    letter-spacing: 8px; padding: 14px;
}
QComboBox {
    background-color: #1e2130; border: 1px solid #334155;
    border-radius: 8px; padding: 8px 12px;
    color: #f1f5f9; font-size: 13px;
}
QComboBox:focus { border: 1.5px solid #3b82f6; }
QComboBox::drop-down { border: none; width: 24px; }
QComboBox QAbstractItemView {
    background-color: #1e2130; border: 1px solid #334155;
    selection-background-color: #2563eb; color: #f1f5f9; padding: 4px;
}
QPushButton {
    background-color: #2563eb; color: #ffffff;
    border: none; border-radius: 8px;
    padding: 10px 20px; font-size: 13px; font-weight: 600;
}
QPushButton:hover { background-color: #3b82f6; }
QPushButton:pressed { background-color: #1d4ed8; }
QPushButton:disabled { background-color: #1e2130; color: #475569; }
QPushButton#dangerBtn { background-color: #dc2626; }
QPushButton#dangerBtn:hover { background-color: #ef4444; }
QPushButton#successBtn { background-color: #16a34a; }
QPushButton#successBtn:hover { background-color: #22c55e; }
QPushButton#warnBtn { background-color: #d97706; }
QPushButton#warnBtn:hover { background-color: #f59e0b; }
QPushButton#logoutBtn {
    background-color: transparent; color: #f87171;
    border: 1px solid #f87171; border-radius: 8px; padding: 10px 16px;
}
QPushButton#logoutBtn:hover { background-color: #f87171; color: #fff; }
QTableWidget {
    background-color: #13161f; border: 1px solid #1e2640;
    border-radius: 8px; gridline-color: #1e2640;
    color: #e2e8f0; alternate-background-color: #181c27;
}
QTableWidget::item { padding: 8px 12px; border: none; }
QTableWidget::item:selected { background-color: #1e3a6e; color: #fff; }
QHeaderView::section {
    background-color: #1a2035; color: #94a3b8;
    padding: 10px 12px; border: none;
    border-bottom: 1px solid #1e2640;
    font-weight: 600; font-size: 11px; letter-spacing: 0.5px;
}
QTabWidget::pane {
    border: 1px solid #1e2640;
    border-radius: 0px 8px 8px 8px;
    background-color: #13161f;
}
QTabBar::tab {
    background-color: #1a2035; color: #64748b;
    padding: 10px 20px;
    border: 1px solid #1e2640; border-bottom: none;
    border-top-left-radius: 8px; border-top-right-radius: 8px;
    margin-right: 2px; font-weight: 500;
}
QTabBar::tab:selected {
    background-color: #13161f; color: #60a5fa;
    border-bottom: 1px solid #13161f; font-weight: 600;
}
QTabBar::tab:hover:!selected { background-color: #1e2540; color: #94a3b8; }
QProgressBar {
    border: 1px solid #1e2640; border-radius: 6px;
    background-color: #1a2035; text-align: center;
    color: #f1f5f9; font-size: 11px; font-weight: 600; height: 18px;
}
QProgressBar::chunk {
    border-radius: 6px;
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2563eb,stop:1 #38bdf8);
}
QProgressBar#dangerProg::chunk {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #dc2626,stop:1 #f97316);
}
QProgressBar#warnProg::chunk {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #d97706,stop:1 #facc15);
}
QScrollBar:vertical { background: #13161f; width: 8px; border-radius: 4px; }
QScrollBar::handle:vertical { background: #334155; border-radius: 4px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: #475569; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
QScrollBar:horizontal { background: #13161f; height: 8px; border-radius: 4px; }
QScrollBar::handle:horizontal { background: #334155; border-radius: 4px; min-width: 30px; }
QFrame#separator { background-color: #1e2640; max-height: 1px; }
QFrame#card {
    background-color: #13161f; border: 1px solid #1e2640;
    border-radius: 12px; padding: 16px;
}
QGroupBox {
    border: 1px solid #1e2640; border-radius: 10px;
    margin-top: 12px; padding: 12px;
    color: #64748b; font-size: 11px; font-weight: 600;
}
QGroupBox::title {
    subcontrol-origin: margin; subcontrol-position: top left;
    padding: 0 8px; color: #64748b;
}
QTextEdit {
    background-color: #13161f; border: 1px solid #1e2640;
    border-radius: 8px; color: #94a3b8;
    font-family: 'Consolas','Courier New',monospace;
    font-size: 12px; padding: 8px;
}
QScrollArea { border: none; background: transparent; }
QScrollArea > QWidget > QWidget { background: transparent; }
QListWidget {
    background-color: #13161f; border: 1px solid #1e2640;
    border-radius: 8px; color: #e2e8f0; outline: 0;
}
QListWidget::item { padding: 10px 12px; border-bottom: 1px solid #1a2035; }
QListWidget::item:selected { background-color: #1e3a6e; color: #fff; }
QListWidget::item:hover { background-color: #1e2540; }
QToolTip {
    background-color: #1e2130; color: #f1f5f9;
    border: 1px solid #334155; border-radius: 6px;
    padding: 6px 10px; font-size: 12px;
}
        )";
    }

    static QString getAttendanceBadgeId(double pct) {
        if (pct >= ATTENDANCE_THRESHOLD) return "badge_ok";
        if (pct >= 60.0)                return "badge_warn";
        return "badge_err";
    }
    static QString getAttendanceText(double pct) {
        if (pct >= ATTENDANCE_THRESHOLD) return "ELIGIBLE";
        if (pct >= 60.0)                return "AT RISK";
        return "INELIGIBLE";
    }
    static QString getProgressBarId(double pct) {
        if (pct >= ATTENDANCE_THRESHOLD) return "";
        if (pct >= 60.0)                return "warnProg";
        return "dangerProg";
    }
};

// ─────────────────────────────────────────────
//  CORE ENGINE  (Singleton)
// ─────────────────────────────────────────────
class CoreEngine {
public:
    static CoreEngine& inst() { static CoreEngine e; return e; }

    User*                   currentUser = nullptr;
    QMap<QString, User>     users;
    QMap<QString, Course>   courses;
    QMap<QString, Session>  activeSessions;   // key = courseCode
    QList<AttendanceRecord> attendanceRecords;
    QList<AdmitCard>        admitCards;
    QList<AuditLog>         auditLogs;

    // ── Utility ──────────────────────────────
    QString hashStr(const QString& s) const {
        return QString(QCryptographicHash::hash(s.toUtf8(),
                                                QCryptographicHash::Sha256).toHex());
    }
    QString nowStr() const {
        return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    }

    // ── Persistence ───────────────────────────
    QString dataPath() const {
        QString p = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(p);
        return p + "/gehu_data.json";
    }

    void save() {
        QJsonObject root;
        QJsonArray ua; for (const auto& u : users)            ua.append(u.toJson());
        QJsonArray ca; for (const auto& c : courses)          ca.append(c.toJson());
        QJsonArray sa; for (const auto& s : activeSessions)   sa.append(s.toJson());
        QJsonArray ra; for (const auto& r : attendanceRecords) ra.append(r.toJson());
        QJsonArray aa; for (const auto& a : admitCards)       aa.append(a.toJson());
        QJsonArray la; for (const auto& l : auditLogs)        la.append(l.toJson());
        root["users"] = ua; root["courses"] = ca; root["sessions"] = sa;
        root["attendance"] = ra; root["admitCards"] = aa; root["auditLogs"] = la;
        root["version"] = APP_VERSION;
        QFile f(dataPath());
        if (f.open(QIODevice::WriteOnly)) {
            f.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
        }
    }

    void load() {
        QFile f(dataPath());
        bool hasData = false;
        if (f.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
            f.close();
            if (!doc.isNull()) {
                QJsonObject root = doc.object();
                hasData = root.contains("users");
                users.clear();
                for (const auto& v : root["users"].toArray()) {
                    User u = User::fromJson(v.toObject()); users[u.username] = u;
                }
                courses.clear();
                for (const auto& v : root["courses"].toArray()) {
                    Course c = Course::fromJson(v.toObject()); courses[c.code] = c;
                }
                activeSessions.clear();
                for (const auto& v : root["sessions"].toArray()) {
                    Session s = Session::fromJson(v.toObject());
                    if (s.isActive && QDateTime::currentSecsSinceEpoch() < s.expiryTime)
                        activeSessions[s.courseCode] = s;
                }
                attendanceRecords.clear();
                for (const auto& v : root["attendance"].toArray())
                    attendanceRecords.append(AttendanceRecord::fromJson(v.toObject()));
                admitCards.clear();
                for (const auto& v : root["admitCards"].toArray())
                    admitCards.append(AdmitCard::fromJson(v.toObject()));
                auditLogs.clear();
                for (const auto& v : root["auditLogs"].toArray())
                    auditLogs.append(AuditLog::fromJson(v.toObject()));
            }
        }
        // Ensure admin always exists
        if (!users.contains("admin")) {
            User a; a.username = "admin"; a.fullName = "System Administrator";
            a.email = "admin@gehu.ac.in"; a.passwordHash = hashStr("admin123");
            a.role = Role::Admin; a.createdAt = nowStr();
            users["admin"] = a;
        }
        if (!hasData) { seedDemoData(); }
        save();
    }

    void seedDemoData() {
        // Teachers
        auto addT = [&](const QString& u, const QString& n, const QString& e) {
            User usr; usr.username=u; usr.fullName=n; usr.email=e;
            usr.passwordHash=hashStr("teacher123"); usr.role=Role::Teacher; usr.createdAt=nowStr();
            users[u]=usr;
        };
        addT("drsingh",  "Dr. Rajesh Singh",   "rajesh.singh@gehu.ac.in");
        addT("profkaur", "Prof. Amrita Kaur",  "amrita.kaur@gehu.ac.in");
        addT("profverma", "Prof. Rohan Verma", "rohan.verma@gehu.ac.in");

        // Students
        auto addS = [&](const QString& u, const QString& n, const QString& e) {
            User usr; usr.username=u; usr.fullName=n; usr.email=e;
            usr.passwordHash=hashStr("student123"); usr.role=Role::Student; usr.createdAt=nowStr();
            users[u]=usr;
        };
        addS("stu001","Arjun Sharma","arjun.sharma@gehu.ac.in");
        addS("stu002","Priya Verma", "priya.verma@gehu.ac.in");
        addS("stu003","Karan Mehta", "karan.mehta@gehu.ac.in");

        // Courses
        auto addC = [&](const QString& code, const QString& name,
                        const QString& teacher, const QString& sem) {
            Course c; c.code=code; c.name=name; c.teacherUsername=teacher;
            c.semester=sem; c.createdAt=nowStr();
            c.description="Core "+name+" curriculum for "+sem;
            courses[code]=c;
        };
        addC("CS101","Data Structures & Algorithms","drsingh",  "Sem 3");
        addC("CS102","Operating Systems",            "drsingh",  "Sem 3");
        addC("CS201","Database Management Systems",  "profkaur", "Sem 4");
        addC("CS301","Computer Networks",            "profverma", "Sem 5");

        // Attendance Records
        struct S { QString stu,crs; int att,tot; };
        QList<S> seeds = {
                           {"stu001","CS101",28,30},{"stu001","CS102",22,30},{"stu001","CS201",18,25},{"stu001","CS301",21,25},
                           {"stu002","CS101",25,30},{"stu002","CS102",29,30},{"stu002","CS201",24,25},{"stu002","CS301",18,25},
                           {"stu003","CS101",20,30},{"stu003","CS102",18,30},{"stu003","CS201",16,25},{"stu003","CS301",22,25},
                           };
        for (const auto& s : seeds) {
            AttendanceRecord r; r.studentUsername=s.stu; r.courseCode=s.crs;
            r.attendedSessions=s.att; r.totalSessions=s.tot;
            attendanceRecords.append(r);
        }

        // Demo medical exemptions for showcasing the eligibility workflow.
        for (auto& r : attendanceRecords) {
            if (r.studentUsername == "stu001" && r.courseCode == "CS101")
                r.medicalExemptions = 1;
            if (r.studentUsername == "stu002" && r.courseCode == "CS201")
                r.medicalExemptions = 1;
            if (r.studentUsername == "stu003" && r.courseCode == "CS301")
                r.medicalExemptions = 2;
        }

        AuditLog l; l.timestamp=QDateTime::currentSecsSinceEpoch();
        l.actor="SYSTEM"; l.action="INIT"; l.details="Demo data seeded.";
        auditLogs.append(l);
    }

    // ── Auth ──────────────────────────────────
    bool login(const QString& username, const QString& password) {
        QString u = username.trimmed().toLower();
        if (!users.contains(u)) return false;
        User& usr = users[u];
        if (!usr.isActive) return false;
        if (usr.passwordHash != hashStr(password)) return false;
        currentUser = &users[u];
        logAudit("LOGIN","User logged in successfully.");
        return true;
    }

    void logout() {
        if (currentUser) logAudit("LOGOUT","User logged out.");
        currentUser = nullptr;
    }

    void logAudit(const QString& action, const QString& details) {
        AuditLog l;
        l.timestamp = QDateTime::currentSecsSinceEpoch();
        l.actor     = currentUser ? currentUser->username : "SYSTEM";
        l.action    = action;
        l.details   = details;
        auditLogs.append(l);
        while (auditLogs.size() > 2000) auditLogs.removeFirst();
        save();
    }

    // ── Admin: Users ──────────────────────────
    bool createUser(const QString& username, const QString& fullName,
                    const QString& email, const QString& password, Role role) {
        if (!currentUser || currentUser->role != Role::Admin) return false;
        QString u = username.trimmed().toLower();
        if (u.isEmpty() || users.contains(u)) return false;
        User usr; usr.username=u; usr.fullName=fullName.trimmed();
        usr.email=email.trimmed(); usr.passwordHash=hashStr(password);
        usr.role=role; usr.createdAt=nowStr();
        users[u]=usr;
        logAudit("CREATE_USER","Created: "+u+" ["+
                                    (role==Role::Teacher?"Teacher":"Student")+"]");
        return true;
    }

    bool toggleUser(const QString& username) {
        if (!currentUser || currentUser->role != Role::Admin) return false;
        if (username=="admin" || !users.contains(username)) return false;
        users[username].isActive = !users[username].isActive;
        logAudit("TOGGLE_USER", username+" → "+(users[username].isActive?"Active":"Inactive"));
        save(); return true;
    }

    bool resetPassword(const QString& username, const QString& newPass) {
        if (!currentUser || currentUser->role != Role::Admin) return false;
        if (!users.contains(username)) return false;
        users[username].passwordHash = hashStr(newPass);
        logAudit("RESET_PASSWORD","Reset for: "+username);
        save(); return true;
    }

    // ── Admin: Courses ────────────────────────
    bool createCourse(const QString& code, const QString& name,
                      const QString& description, const QString& teacher,
                      const QString& semester) {
        if (!currentUser || currentUser->role != Role::Admin) return false;
        QString c = code.trimmed().toUpper();
        if (c.isEmpty() || courses.contains(c)) return false;
        if (!users.contains(teacher) || users[teacher].role != Role::Teacher) return false;
        Course crs; crs.code=c; crs.name=name.trimmed(); crs.description=description.trimmed();
        crs.teacherUsername=teacher; crs.semester=semester.trimmed(); crs.createdAt=nowStr();
        courses[c]=crs;
        logAudit("CREATE_COURSE","Created: "+c+" → "+teacher);
        save(); return true;
    }

    // ── Admin: Admit Cards ────────────────────
    void generateAdmitCards(const QString& courseCode, const QString& examDate) {
        if (!currentUser || currentUser->role != Role::Admin) return;
        if (!courses.contains(courseCode)) return;
        // Remove old cards for this course
        admitCards.erase(
            std::remove_if(admitCards.begin(), admitCards.end(),
                           [&](const AdmitCard& ac){ return ac.courseCode == courseCode; }),
            admitCards.end());
        int seatNum = 101;
        for (const auto& rec : attendanceRecords) {
            if (rec.courseCode != courseCode) continue;
            double pct = calcAttendance(rec);
            AdmitCard card;
            card.cardId = "AC-"+courseCode+"-"+rec.studentUsername+"-"
                          +QString::number(QDateTime::currentMSecsSinceEpoch()).right(6);
            card.studentUsername = rec.studentUsername;
            card.studentFullName = users.contains(rec.studentUsername)
                                       ? users[rec.studentUsername].fullName
                                       : rec.studentUsername;
            card.courseCode  = courseCode;
            card.approved    = (pct >= ATTENDANCE_THRESHOLD);
            card.seatNumber  = card.approved ? "SEAT-"+QString::number(seatNum++) : "N/A";
            card.examDate    = examDate;
            card.generatedAt = nowStr();
            QString payload  = card.cardId+card.studentUsername+card.courseCode
                              +(card.approved?"1":"0")+card.seatNumber+examDate;
            card.signature   = hashStr(payload).left(32);
            admitCards.append(card);
        }
        logAudit("GEN_ADMIT_CARDS","Generated for: "+courseCode);
        save();
    }

    // ── Teacher: Sessions ─────────────────────
    QString startSession(const QString& courseCode) {
        if (!currentUser || currentUser->role != Role::Teacher) return {};
        QString cc = courseCode.trimmed().toUpper();
        if (!courses.contains(cc)) return {};
        if (courses[cc].teacherUsername != currentUser->username) return {};
        Session s;
        s.sessionId       = "SES-"+cc+"-"+QString::number(QDateTime::currentMSecsSinceEpoch());
        s.courseCode      = cc; s.teacherUsername = currentUser->username; s.isActive = true;
        s.startTime       = QDateTime::currentSecsSinceEpoch();
        s.expiryTime      = s.startTime + OTP_LIFETIME_SECONDS;
        const QString chars("ABCDEFGHJKLMNPQRSTUVWXYZ23456789");
        for (int i=0; i<OTP_LENGTH; ++i) s.otp += chars.at(QRandomGenerator::global()->bounded(chars.length()));
        activeSessions[cc] = s;
        for (const auto& user : users) {
            if (user.role != Role::Student || !user.isActive) continue;
            AttendanceRecord* rec = nullptr;
            for (auto& existing : attendanceRecords) {
                if (existing.studentUsername == user.username && existing.courseCode == cc) {
                    rec = &existing;
                    break;
                }
            }
            if (!rec) {
                AttendanceRecord fresh;
                fresh.studentUsername = user.username;
                fresh.courseCode = cc;
                fresh.totalSessions = 1;
                attendanceRecords.append(fresh);
            } else {
                rec->totalSessions++;
            }
        }
        logAudit("START_SESSION","Attendance session started for "+cc);
        save(); return s.otp;
    }

    void endSession(const QString& courseCode) {
        QString cc = courseCode.trimmed().toUpper();
        if (activeSessions.contains(cc)) {
            activeSessions.remove(cc);
            logAudit("END_SESSION","Session ended: "+cc);
            save();
        }
    }

    bool isSessionActive(const QString& courseCode) const {
        QString cc = courseCode.trimmed().toUpper();
        if (!activeSessions.contains(cc)) return false;
        return QDateTime::currentSecsSinceEpoch() < activeSessions[cc].expiryTime;
    }

    int sessionSecondsLeft(const QString& courseCode) const {
        QString cc = courseCode.trimmed().toUpper();
        if (!activeSessions.contains(cc)) return 0;
        qint64 left = activeSessions[cc].expiryTime - QDateTime::currentSecsSinceEpoch();
        return static_cast<int>(qMax((qint64)0, left));
    }

    QList<QString> getTeacherCourses(const QString& teacherUsername) const {
        QList<QString> result;
        for (const auto& c : courses)
            if (c.teacherUsername == teacherUsername && c.isActive)
                result.append(c.code);
        return result;
    }

    QList<AttendanceRecord> getCourseAttendance(const QString& courseCode) const {
        QList<AttendanceRecord> result;
        for (const auto& r : attendanceRecords)
            if (r.courseCode == courseCode) result.append(r);
        return result;
    }

    // ── Student: Attendance ───────────────────
    bool submitAttendance(const QString& courseCode, const QString& otp) {
        if (!currentUser || currentUser->role != Role::Student) return false;
        QString cc = courseCode.trimmed().toUpper();
        if (!isSessionActive(cc)) return false;
        const Session& s = activeSessions[cc];
        if (s.otp.toUpper() != otp.trimmed().toUpper()) return false;
        // Find or create record
        AttendanceRecord* rec = nullptr;
        for (auto& r : attendanceRecords)
            if (r.studentUsername == currentUser->username && r.courseCode == cc)
            { rec = &r; break; }
        if (!rec) {
            AttendanceRecord r; r.studentUsername=currentUser->username;
            r.courseCode=cc; r.totalSessions=1;
            attendanceRecords.append(r);
            rec = &attendanceRecords.last();
        }
        // Block duplicate submission
        if (rec->attendedSessionIds.contains(s.sessionId)) return false;
        rec->attendedSessions++;
        rec->attendedSessionIds.append(s.sessionId);
        logAudit("SUBMIT_ATTENDANCE",currentUser->username+" attended "+cc);
        save(); return true;
    }

    QList<AttendanceRecord> getStudentAttendance(const QString& username) const {
        QList<AttendanceRecord> result;
        for (const auto& r : attendanceRecords)
            if (r.studentUsername == username) result.append(r);
        return result;
    }

    QList<AdmitCard> getStudentAdmitCards(const QString& username) const {
        QList<AdmitCard> result;
        for (const auto& c : admitCards)
            if (c.studentUsername == username) result.append(c);
        return result;
    }

    // ── Admin/Teacher: Medical Attendance Exemption ─────────
    bool grantMedicalExemption(const QString& studentUsername,
                               const QString& courseCode,
                               int sessions) {
        if (!currentUser) return false;
        if (currentUser->role != Role::Admin &&
            currentUser->role != Role::Teacher) return false;
        if (sessions <= 0) return false;

        QString student = studentUsername.trimmed().toLower();
        QString course = courseCode.trimmed().toUpper();

        if (!users.contains(student) ||
            users[student].role != Role::Student) return false;
        if (!courses.contains(course)) return false;

        if (currentUser->role == Role::Teacher &&
            courses[course].teacherUsername != currentUser->username) return false;

        for (auto& rec : attendanceRecords) {
            if (rec.studentUsername == student && rec.courseCode == course) {
                int remaining = rec.totalSessions - rec.attendedSessions;
                if (remaining < 0 || sessions > remaining) return false;

                rec.medicalExemptions += sessions;
                logAudit("MEDICAL_EXEMPTION",
                         "Granted " + QString::number(sessions) +
                             " exemption(s) to: " + student +
                             " [" + course + "]");
                save();
                return true;
            }
        }
        return false;
    }

    double calcAttendance(const AttendanceRecord& r) const {
        if (r.totalSessions <= 0) return 0.0;
        return ((r.attendedSessions + r.medicalExemptions) / (double)r.totalSessions) * 100.0;
    }

    // ── Stats ─────────────────────────────────
    int countByRole(Role role) const {
        int n=0; for (const auto& u : users) if (u.role==role && u.isActive) n++; return n;
    }
    double overallAvgAttendance() const {
        if (attendanceRecords.isEmpty()) return 0.0;
        double sum=0;
        for (const auto& r : attendanceRecords) sum += calcAttendance(r);
        return sum / attendanceRecords.size();
    }
};

// ─────────────────────────────────────────────
//  HELPER BUILDERS
// ─────────────────────────────────────────────
static QFrame* makeSeparator(QWidget* parent=nullptr) {
    auto* f = new QFrame(parent);
    f->setObjectName("separator");
    f->setFrameShape(QFrame::HLine);
    f->setFixedHeight(1);
    return f;
}
static QLabel* makeTitle(const QString& text, QWidget* parent=nullptr) {
    auto* l = new QLabel(text, parent);
    l->setObjectName("titleLabel");
    return l;
}
static QLabel* makeSectionHeader(const QString& text, QWidget* parent=nullptr) {
    auto* l = new QLabel(text, parent);
    l->setObjectName("sectionHeader");
    return l;
}

// ─────────────────────────────────────────────
//  CIRCULAR TIMER WIDGET
// ─────────────────────────────────────────────
class CircularTimer : public QWidget {
public:
    explicit CircularTimer(QWidget* parent=nullptr) : QWidget(parent) { setFixedSize(110,110); }
    int secondsLeft() const { return m_sec; }
    void setSecondsLeft(int s) { m_sec=s; update(); }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        int m=8;
        QRectF rect(m, m, width()-2*m, height()-2*m);
        p.setPen(QPen(QColor("#1e2640"),7,Qt::SolidLine,Qt::FlatCap));
        p.drawEllipse(rect);
        double frac = qBound(0.0, m_sec/(double)OTP_LIFETIME_SECONDS, 1.0);
        QColor c = frac>0.4 ? QColor("#34d399") : (frac>0.15 ? QColor("#fbbf24") : QColor("#f87171"));
        p.setPen(QPen(c,7,Qt::SolidLine,Qt::RoundCap));
        p.drawArc(rect, 90*16, static_cast<int>(frac*360*16));
        p.setPen(c);
        QFont f; f.setPointSize(18); f.setBold(true); p.setFont(f);
        p.drawText(rect, Qt::AlignCenter, QString::number(m_sec));
    }
private:
    int m_sec = OTP_LIFETIME_SECONDS;
};

// ─────────────────────────────────────────────
//  ADMIN DASHBOARD
// ─────────────────────────────────────────────
class NavigationHandler {
public:
    virtual ~NavigationHandler() = default;
    virtual void handleLogin(Role role) = 0;
    virtual void handleLogout() = 0;
};

class AdminDashboard : public QWidget {
public:
    explicit AdminDashboard(QWidget* parent=nullptr) : QWidget(parent) {
        auto* lay = new QVBoxLayout(this);
        lay->setContentsMargins(0,0,0,0);
        m_tabs = new QTabWidget(this);
        m_tabs->addTab(buildOverviewTab(),  "  Overview  ");
        m_tabs->addTab(buildUsersTab(),     "  Users  ");
        m_tabs->addTab(buildCoursesTab(),   "  Courses  ");
        m_tabs->addTab(buildAdmitTab(),     "  Admit Cards  ");
        m_tabs->addTab(buildAuditTab(),     "  Audit Log  ");
        connect(m_tabs, &QTabWidget::currentChanged, this, [this](int idx){
            if (idx==0) refreshOverview();
            if (idx==1) refreshUsers();
            if (idx==2) refreshCourses();
            if (idx==3) refreshAdmitCards();
            if (idx==4) refreshAuditLog();
        });
        lay->addWidget(m_tabs);
        refreshOverview();
    }
    void setNavigationHandler(NavigationHandler* handler) {
        m_navigation = handler;
    }

    void onActivated() { refreshOverview(); }

private:
    NavigationHandler* m_navigation = nullptr;
    // ── All member pointers declared ONCE here ──
    QTabWidget* m_tabs = nullptr;

    // Overview
    QLabel* m_statTeachers = nullptr;
    QLabel* m_statStudents = nullptr;
    QLabel* m_statCourses  = nullptr;
    QLabel* m_statAvg      = nullptr;

    // Users
    QTableWidget* m_usersTable = nullptr;
    QLineEdit* m_uname  = nullptr;
    QLineEdit* m_ufull  = nullptr;
    QLineEdit* m_uemail = nullptr;
    QLineEdit* m_upass  = nullptr;
    QComboBox* m_urole  = nullptr;

    // Courses
    QTableWidget* m_coursesTable = nullptr;
    QLineEdit* m_ccode    = nullptr;
    QLineEdit* m_cname    = nullptr;
    QLineEdit* m_cdesc    = nullptr;
    QLineEdit* m_csem     = nullptr;
    QComboBox* m_cteacher = nullptr;

    // Admit Cards
    QTableWidget* m_admitTable  = nullptr;
    QComboBox*    m_admitCourse = nullptr;   // ← declared ONCE
    QLineEdit*    m_admitDate   = nullptr;   // ← declared ONCE

    // Audit Log
    QTableWidget* m_auditTable = nullptr;

    // ── Overview ──────────────────────────────
    QWidget* buildOverviewTab() {
        auto* w = new QWidget();
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(24,24,24,24); lay->setSpacing(20);

        lay->addWidget(makeTitle("Admin Overview"));
        lay->addWidget(makeSeparator());

        auto* statsRow = new QHBoxLayout();
        auto addStat = [&](QLabel*& label, const QString& title, const QString& color) {
            auto* card = new QFrame(); card->setObjectName("card");
            auto* vl = new QVBoxLayout(card); vl->setContentsMargins(20,20,20,20);
            label = new QLabel("0");
            label->setStyleSheet("color:"+color+";font-size:36px;font-weight:800;");
            auto* tl = new QLabel(title); tl->setObjectName("statTitle");
            vl->addWidget(label); vl->addWidget(tl);
            statsRow->addWidget(card);
        };
        addStat(m_statTeachers,"TEACHERS",       "#38bdf8");
        addStat(m_statStudents,"STUDENTS",        "#34d399");
        addStat(m_statCourses, "ACTIVE COURSES",  "#a78bfa");
        addStat(m_statAvg,     "AVG ATTENDANCE",  "#fbbf24");
        lay->addLayout(statsRow);

        lay->addWidget(makeSectionHeader("Quick Actions"));
        auto* btnRow = new QHBoxLayout();
        auto* addUserBtn   = new QPushButton("+ Add User");
        auto* addCourseBtn = new QPushButton("+ Add Course");
        auto* genBtn       = new QPushButton("⬡ Admit Cards");
        auto* logoutBtn    = new QPushButton("Logout");
        addUserBtn->setObjectName("successBtn");
        addCourseBtn->setObjectName("successBtn");
        genBtn->setObjectName("warnBtn");
        logoutBtn->setObjectName("logoutBtn");
        connect(addUserBtn,   &QPushButton::clicked, this,[this]{m_tabs->setCurrentIndex(1);});
        connect(addCourseBtn, &QPushButton::clicked, this,[this]{m_tabs->setCurrentIndex(2);});
        connect(genBtn,       &QPushButton::clicked, this,[this]{m_tabs->setCurrentIndex(3);});
        connect(logoutBtn, &QPushButton::clicked, this,[this]{
            CoreEngine::inst().logout(); if (m_navigation) m_navigation->handleLogout();
        });
        btnRow->addWidget(addUserBtn); btnRow->addWidget(addCourseBtn);
        btnRow->addWidget(genBtn); btnRow->addStretch(); btnRow->addWidget(logoutBtn);
        lay->addLayout(btnRow);
        lay->addStretch();
        return w;
    }

    void refreshOverview() {
        auto& e = CoreEngine::inst();
        if (m_statTeachers) m_statTeachers->setText(QString::number(e.countByRole(Role::Teacher)));
        if (m_statStudents) m_statStudents->setText(QString::number(e.countByRole(Role::Student)));
        if (m_statCourses)  m_statCourses->setText(QString::number(e.courses.size()));
        if (m_statAvg)      m_statAvg->setText(QString::number(e.overallAvgAttendance(),'f',1)+"%");
    }

    // ── Users ─────────────────────────────────
    QWidget* buildUsersTab() {
        auto* w = new QWidget();
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(24,24,24,24); lay->setSpacing(16);

        lay->addWidget(makeTitle("User Management"));
        lay->addWidget(makeSeparator());

        auto* grp = new QGroupBox("Create New Account");
        auto* gl  = new QGridLayout(grp); gl->setSpacing(10);

        m_uname  = new QLineEdit(); m_uname->setPlaceholderText("Username (e.g. stu004)");
        m_ufull  = new QLineEdit(); m_ufull->setPlaceholderText("Full Name");
        m_uemail = new QLineEdit(); m_uemail->setPlaceholderText("Email");
        m_upass  = new QLineEdit(); m_upass->setPlaceholderText("Password");
        m_upass->setEchoMode(QLineEdit::Password);
        m_urole  = new QComboBox();
        m_urole->addItem("Teacher", static_cast<int>(Role::Teacher));
        m_urole->addItem("Student", static_cast<int>(Role::Student));
        auto* createBtn = new QPushButton("Create Account");
        createBtn->setObjectName("successBtn");

        gl->addWidget(new QLabel("Username:"),0,0); gl->addWidget(m_uname,0,1);
        gl->addWidget(new QLabel("Full Name:"),0,2); gl->addWidget(m_ufull,0,3);
        gl->addWidget(new QLabel("Email:"),1,0);    gl->addWidget(m_uemail,1,1);
        gl->addWidget(new QLabel("Password:"),1,2); gl->addWidget(m_upass,1,3);
        gl->addWidget(new QLabel("Role:"),2,0);     gl->addWidget(m_urole,2,1);
        gl->addWidget(createBtn,2,3);

        connect(createBtn, &QPushButton::clicked, this, &AdminDashboard::onCreateUser);
        lay->addWidget(grp);
        lay->addWidget(makeSectionHeader("All Accounts"));

        m_usersTable = new QTableWidget(0,6);
        m_usersTable->setHorizontalHeaderLabels({"Username","Full Name","Email","Role","Status","Actions"});
        m_usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_usersTable->setAlternatingRowColors(true);
        m_usersTable->verticalHeader()->setVisible(false);
        lay->addWidget(m_usersTable);
        return w;
    }

    void onCreateUser() {
        if (m_uname->text().isEmpty()||m_ufull->text().isEmpty()||m_upass->text().isEmpty()) {
            QMessageBox::warning(this,"Validation","Username, Full Name & Password required."); return;
        }
        Role r = static_cast<Role>(m_urole->currentData().toInt());
        if (CoreEngine::inst().createUser(m_uname->text(),m_ufull->text(),m_uemail->text(),m_upass->text(),r)) {
            QMessageBox::information(this,"Success","Account created.");
            m_uname->clear(); m_ufull->clear(); m_uemail->clear(); m_upass->clear();
            refreshUsers();
        } else {
            QMessageBox::warning(this,"Error","Username already exists or invalid input.");
        }
    }

    void refreshUsers() {
        if (!m_usersTable) return;
        m_usersTable->setRowCount(0);
        auto& e = CoreEngine::inst();
        for (const auto& u : e.users) {
            if (u.role == Role::Admin) continue;
            int row = m_usersTable->rowCount();
            m_usersTable->insertRow(row);
            QString roleStr = (u.role==Role::Teacher) ? "Teacher" : "Student";
            QStringList cells = {u.username,u.fullName,u.email,roleStr,
                                 u.isActive?"Active":"Inactive"};
            for (int i=0; i<cells.size(); ++i) {
                auto* item = new QTableWidgetItem(cells[i]);
                item->setTextAlignment(Qt::AlignCenter);
                if (i==4) item->setForeground(u.isActive ? QColor("#34d399"):QColor("#f87171"));
                m_usersTable->setItem(row,i,item);
            }
            auto* actW = new QWidget();
            auto* al   = new QHBoxLayout(actW);
            al->setContentsMargins(4,2,4,2); al->setSpacing(6);
            auto* togBtn  = new QPushButton(u.isActive?"Deactivate":"Activate");
            auto* rstBtn  = new QPushButton("Reset Pwd");
            togBtn->setObjectName(u.isActive?"dangerBtn":"successBtn");
            togBtn->setFixedHeight(28); rstBtn->setFixedHeight(28);
            togBtn->setStyleSheet(togBtn->styleSheet()+"font-size:11px;padding:2px 8px;");
            rstBtn->setStyleSheet("font-size:11px;padding:2px 8px;");
            QString uname = u.username;
            connect(togBtn,&QPushButton::clicked,this,[this,uname]{
                CoreEngine::inst().toggleUser(uname); refreshUsers();
            });
            connect(rstBtn,&QPushButton::clicked,this,[this,uname]{
                QDialog dlg(this); dlg.setWindowTitle("Reset Password"); dlg.setMinimumWidth(320);
                auto* vl = new QVBoxLayout(&dlg);
                auto* pe = new QLineEdit(&dlg); pe->setPlaceholderText("New Password");
                pe->setEchoMode(QLineEdit::Password);
                auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dlg);
                vl->addWidget(new QLabel("New password for: "+uname));
                vl->addWidget(pe); vl->addWidget(btns);
                connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
                connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
                if (dlg.exec()==QDialog::Accepted && !pe->text().isEmpty()) {
                    CoreEngine::inst().resetPassword(uname,pe->text());
                    QMessageBox::information(this,"Done","Password reset for "+uname+".");
                }
            });
            al->addWidget(togBtn); al->addWidget(rstBtn);
            m_usersTable->setCellWidget(row,5,actW);
            m_usersTable->setRowHeight(row,42);
        }
        refreshAdmitCourseCombo();
    }

    // ── Courses ───────────────────────────────
    QWidget* buildCoursesTab() {
        auto* w = new QWidget();
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(24,24,24,24); lay->setSpacing(16);

        lay->addWidget(makeTitle("Course Management"));
        lay->addWidget(makeSeparator());

        auto* grp = new QGroupBox("Create New Course");
        auto* gl  = new QGridLayout(grp); gl->setSpacing(10);

        m_ccode    = new QLineEdit(); m_ccode->setPlaceholderText("Code (e.g. CS303)");
        m_cname    = new QLineEdit(); m_cname->setPlaceholderText("Course Name");
        m_cdesc    = new QLineEdit(); m_cdesc->setPlaceholderText("Description (optional)");
        m_csem     = new QLineEdit(); m_csem->setPlaceholderText("Semester (e.g. Sem 3)");
        m_cteacher = new QComboBox();
        auto* createBtn = new QPushButton("Create Course");
        createBtn->setObjectName("successBtn");

        gl->addWidget(new QLabel("Code:"),   0,0); gl->addWidget(m_ccode,  0,1);
        gl->addWidget(new QLabel("Name:"),   0,2); gl->addWidget(m_cname,  0,3);
        gl->addWidget(new QLabel("Desc:"),   1,0); gl->addWidget(m_cdesc,  1,1,1,3);
        gl->addWidget(new QLabel("Semester:"),2,0); gl->addWidget(m_csem,  2,1);
        gl->addWidget(new QLabel("Teacher:"),2,2); gl->addWidget(m_cteacher,2,3);
        gl->addWidget(createBtn,3,3);

        connect(createBtn, &QPushButton::clicked, this, &AdminDashboard::onCreateCourse);
        lay->addWidget(grp);
        lay->addWidget(makeSectionHeader("All Courses"));

        m_coursesTable = new QTableWidget(0,5);
        m_coursesTable->setHorizontalHeaderLabels({"Code","Name","Semester","Teacher","Enrolled"});
        m_coursesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_coursesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_coursesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_coursesTable->setAlternatingRowColors(true);
        m_coursesTable->verticalHeader()->setVisible(false);
        lay->addWidget(m_coursesTable);
        return w;
    }

    void refreshTeacherCombo() {
        if (!m_cteacher) return;
        m_cteacher->clear();
        for (const auto& u : CoreEngine::inst().users)
            if (u.role==Role::Teacher && u.isActive)
                m_cteacher->addItem(u.fullName+" ("+u.username+")", u.username);
    }

    void onCreateCourse() {
        if (m_ccode->text().isEmpty()||m_cname->text().isEmpty()||m_cteacher->currentIndex()<0) {
            QMessageBox::warning(this,"Validation","Code, Name & Teacher are required."); return;
        }
        QString teacher = m_cteacher->currentData().toString();
        if (CoreEngine::inst().createCourse(m_ccode->text(),m_cname->text(),
                                            m_cdesc->text(),teacher,m_csem->text())) {
            QMessageBox::information(this,"Success","Course created.");
            m_ccode->clear(); m_cname->clear(); m_cdesc->clear(); m_csem->clear();
            refreshCourses();
        } else {
            QMessageBox::warning(this,"Error","Code already exists or teacher not found.");
        }
    }

    void refreshCourses() {
        if (!m_coursesTable) return;
        refreshTeacherCombo();
        m_coursesTable->setRowCount(0);
        auto& e = CoreEngine::inst();
        for (const auto& c : e.courses) {
            int enrolled = e.getCourseAttendance(c.code).size();
            int row = m_coursesTable->rowCount();
            m_coursesTable->insertRow(row);
            QString tname = e.users.contains(c.teacherUsername)
                                ? e.users[c.teacherUsername].fullName : c.teacherUsername;
            QStringList cells = {c.code,c.name,c.semester,tname,QString::number(enrolled)};
            for (int i=0; i<cells.size(); ++i) {
                auto* item = new QTableWidgetItem(cells[i]);
                item->setTextAlignment(Qt::AlignCenter);
                m_coursesTable->setItem(row,i,item);
            }
            m_coursesTable->setRowHeight(row,38);
        }
        refreshAdmitCourseCombo();
    }

    // ── Admit Cards ───────────────────────────
    QWidget* buildAdmitTab() {
        auto* w = new QWidget();
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(24,24,24,24); lay->setSpacing(16);

        lay->addWidget(makeTitle("Admit Card Generator"));
        lay->addWidget(makeSeparator());

        auto* grp = new QGroupBox("Generate Admit Cards");
        auto* gl  = new QHBoxLayout(grp);
        m_admitCourse = new QComboBox();    // initialise the member pointer
        m_admitDate   = new QLineEdit();    // initialise the member pointer
        m_admitDate->setPlaceholderText("Exam Date (e.g. 2026-11-15)");
        auto* genBtn = new QPushButton("Generate Cards");
        genBtn->setObjectName("warnBtn");
        gl->addWidget(new QLabel("Course:")); gl->addWidget(m_admitCourse);
        gl->addWidget(new QLabel("Exam Date:")); gl->addWidget(m_admitDate);
        gl->addWidget(genBtn);

        connect(genBtn, &QPushButton::clicked, this, [this](){
            if (!m_admitCourse || m_admitCourse->currentIndex()<0 || m_admitDate->text().isEmpty()) {
                QMessageBox::warning(this,"Validation","Select a course and enter exam date."); return;
            }
            auto& e = CoreEngine::inst();
            QString cc = m_admitCourse->currentData().toString();
            if (e.getCourseAttendance(cc).isEmpty()) {
                QMessageBox::information(this,"Info","No attendance records for this course."); return;
            }
            e.generateAdmitCards(cc, m_admitDate->text());
            QMessageBox::information(this,"Done","Admit cards generated successfully.");
            refreshAdmitCards();
        });

        lay->addWidget(grp);
        lay->addWidget(makeSectionHeader("Generated Admit Cards"));

        m_admitTable = new QTableWidget(0,7);
        m_admitTable->setHorizontalHeaderLabels(
            {"Card ID","Student","Course","Seat","Exam Date","Status","Signature"});
        m_admitTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_admitTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_admitTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_admitTable->setAlternatingRowColors(true);
        m_admitTable->verticalHeader()->setVisible(false);
        lay->addWidget(m_admitTable);
        return w;
    }

    void refreshAdmitCourseCombo() {
        if (!m_admitCourse) return;     // ← null-guard (FIX)
        m_admitCourse->clear();
        for (const auto& c : CoreEngine::inst().courses)
            m_admitCourse->addItem(c.code+" — "+c.name, c.code);
    }

    void refreshAdmitCards() {
        if (!m_admitTable) return;
        m_admitTable->setRowCount(0);
        for (const auto& card : CoreEngine::inst().admitCards) {
            int row = m_admitTable->rowCount();
            m_admitTable->insertRow(row);
            QStringList cells = {card.cardId.right(16),card.studentFullName,
                                 card.courseCode,card.seatNumber,card.examDate,
                                 card.approved?"ELIGIBLE":"BLOCKED",
                                 card.signature.left(16)+"..."};
            for (int i=0; i<cells.size(); ++i) {
                auto* item = new QTableWidgetItem(cells[i]);
                item->setTextAlignment(Qt::AlignCenter);
                if (i==5) item->setForeground(card.approved?QColor("#34d399"):QColor("#f87171"));
                m_admitTable->setItem(row,i,item);
            }
            m_admitTable->setRowHeight(row,38);
        }
    }

    // ── Audit Log ─────────────────────────────
    QWidget* buildAuditTab() {
        auto* w = new QWidget();
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(24,24,24,24); lay->setSpacing(16);

        lay->addWidget(makeTitle("Audit Log"));
        lay->addWidget(makeSeparator());

        m_auditTable = new QTableWidget(0,4);
        m_auditTable->setHorizontalHeaderLabels({"Timestamp","Actor","Action","Details"});
        m_auditTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_auditTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_auditTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_auditTable->setAlternatingRowColors(true);
        m_auditTable->verticalHeader()->setVisible(false);
        lay->addWidget(m_auditTable);
        return w;
    }

    void refreshAuditLog() {
        if (!m_auditTable) return;
        m_auditTable->setRowCount(0);
        auto& e = CoreEngine::inst();
        for (int i = e.auditLogs.size()-1; i>=0; --i) {
            const auto& log = e.auditLogs[i];
            int row = m_auditTable->rowCount();
            m_auditTable->insertRow(row);
            QString ts = QDateTime::fromSecsSinceEpoch(log.timestamp)
                             .toString("yyyy-MM-dd hh:mm:ss");
            QStringList cells = {ts, log.actor, log.action, log.details};
            for (int j=0; j<cells.size(); ++j) {
                auto* item = new QTableWidgetItem(cells[j]);
                item->setTextAlignment(j==3 ? Qt::AlignLeft|Qt::AlignVCenter : Qt::AlignCenter);
                m_auditTable->setItem(row,j,item);
            }
            m_auditTable->setRowHeight(row,36);
        }
    }
};

// ─────────────────────────────────────────────
//  TEACHER DASHBOARD
// ─────────────────────────────────────────────
class TeacherDashboard : public QWidget {
public:
    explicit TeacherDashboard(QWidget* parent=nullptr) : QWidget(parent) {
        m_ticker = new QTimer(this);
        m_ticker->setInterval(1000);
        connect(m_ticker, &QTimer::timeout, this, &TeacherDashboard::onTick);
        m_ticker->start();

        auto* lay = new QVBoxLayout(this);
        lay->setContentsMargins(0,0,0,0);
        m_tabs = new QTabWidget(this);
        m_tabs->addTab(buildSessionTab(),    "  Session  ");
        m_tabs->addTab(buildAttendanceTab(), "  Attendance  ");
        m_tabs->addTab(buildCoursesTab(),    "  My Courses  ");
        connect(m_tabs,&QTabWidget::currentChanged,this,[this](int idx){
            if (idx==1) refreshAttendance();
            if (idx==2) refreshMyCourses();
        });
        lay->addWidget(m_tabs);
    }
    void setNavigationHandler(NavigationHandler* handler) {
        m_navigation = handler;
    }

    void onActivated() {
        refreshCourseCombo();
        refreshMyCourses();
        refreshAttendance();
    }

private:
    NavigationHandler* m_navigation = nullptr;
    QTabWidget* m_tabs        = nullptr;
    QTimer*     m_ticker      = nullptr;
    QString     m_activeCourse;

    // Session tab
    QComboBox*     m_courseCombo  = nullptr;
    QLabel*        m_otpLabel     = nullptr;
    QLabel*        m_sessionStat  = nullptr;
    CircularTimer* m_circTimer    = nullptr;
    QPushButton*   m_startBtn     = nullptr;
    QPushButton*   m_stopBtn      = nullptr;

    // Attendance tab
    QComboBox*    m_attCourse = nullptr;
    QTableWidget* m_attTable  = nullptr;

    // Courses tab
    QTableWidget* m_coursesTable = nullptr;

    QWidget* buildSessionTab() {
        auto* w = new QWidget();
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(32,32,32,32); lay->setSpacing(20);

        lay->addWidget(makeTitle("Attendance Session"));
        lay->addWidget(makeSeparator());

        auto* row1 = new QHBoxLayout();
        row1->addWidget(new QLabel("Select Course:"));
        m_courseCombo = new QComboBox(); m_courseCombo->setMinimumWidth(280);
        row1->addWidget(m_courseCombo); row1->addStretch();
        lay->addLayout(row1);

        // OTP + circular timer
        auto* otpRow = new QHBoxLayout();
        otpRow->setAlignment(Qt::AlignCenter);
        m_otpLabel = new QLabel("------");
        m_otpLabel->setObjectName("otpDisplay");
        m_otpLabel->setAlignment(Qt::AlignCenter);
        m_circTimer = new CircularTimer();
        otpRow->addStretch();
        otpRow->addWidget(m_otpLabel);
        otpRow->addSpacing(32);
        otpRow->addWidget(m_circTimer);
        otpRow->addStretch();
        lay->addLayout(otpRow);

        m_sessionStat = new QLabel("No active session.");
        m_sessionStat->setAlignment(Qt::AlignCenter);
        m_sessionStat->setStyleSheet("color:#64748b;font-size:13px;");
        lay->addWidget(m_sessionStat);

        auto* btnRow = new QHBoxLayout();
        m_startBtn = new QPushButton("▶  Start Session");
        m_startBtn->setObjectName("successBtn"); m_startBtn->setFixedHeight(44);
        m_stopBtn  = new QPushButton("■  End Session");
        m_stopBtn->setObjectName("dangerBtn"); m_stopBtn->setFixedHeight(44);
        m_stopBtn->setEnabled(false);
        auto* logoutBtn = new QPushButton("Logout"); logoutBtn->setObjectName("logoutBtn");
        connect(m_startBtn, &QPushButton::clicked, this, &TeacherDashboard::onStart);
        connect(m_stopBtn,  &QPushButton::clicked, this, &TeacherDashboard::onStop);
        connect(logoutBtn,  &QPushButton::clicked, this,[this]{
            onStop();
            CoreEngine::inst().logout();
            if (m_navigation) m_navigation->handleLogout();
        });
        btnRow->addWidget(m_startBtn); btnRow->addWidget(m_stopBtn);
        btnRow->addStretch(); btnRow->addWidget(logoutBtn);
        lay->addLayout(btnRow);
        lay->addStretch();
        return w;
    }

    void refreshCourseCombo() {
        if (!m_courseCombo) return;
        m_courseCombo->clear();
        auto& e = CoreEngine::inst();
        if (!e.currentUser) return;
        auto codes = e.getTeacherCourses(e.currentUser->username);
        for (const auto& code : codes) {
            const auto& c = e.courses[code];
            m_courseCombo->addItem(code+" — "+c.name, code);
        }
        if (m_attCourse) {
            m_attCourse->clear();
            for (const auto& code : codes) {
                const auto& c = e.courses[code];
                m_attCourse->addItem(code+" — "+c.name, code);
            }
        }
    }

    void onStart() {
        if (!m_courseCombo || m_courseCombo->currentIndex()<0) {
            QMessageBox::warning(this,"Error","No course selected."); return;
        }
        m_activeCourse = m_courseCombo->currentData().toString();
        QString otp = CoreEngine::inst().startSession(m_activeCourse);
        if (otp.isEmpty()) {
            QMessageBox::warning(this,"Error","Cannot start session. Course must be assigned to you.");
            m_activeCourse.clear(); return;
        }
        m_otpLabel->setText(otp);
        m_otpLabel->setObjectName("otpDisplay");
        m_otpLabel->style()->unpolish(m_otpLabel); m_otpLabel->style()->polish(m_otpLabel);
        m_startBtn->setEnabled(false);
        m_stopBtn->setEnabled(true);
        m_courseCombo->setEnabled(false);
    }

    void onStop() {
        if (!m_activeCourse.isEmpty()) {
            CoreEngine::inst().endSession(m_activeCourse);
            m_activeCourse.clear();
        }
        if (!m_otpLabel) return;
        m_otpLabel->setText("------");
        m_otpLabel->setObjectName("otpExpired");
        m_otpLabel->style()->unpolish(m_otpLabel); m_otpLabel->style()->polish(m_otpLabel);
        if (m_sessionStat) m_sessionStat->setText("Session ended.");
        if (m_startBtn)    m_startBtn->setEnabled(true);
        if (m_stopBtn)     m_stopBtn->setEnabled(false);
        if (m_courseCombo) m_courseCombo->setEnabled(true);
        if (m_circTimer)   m_circTimer->setSecondsLeft(0);
    }

    void onTick() {
        if (m_activeCourse.isEmpty()) return;
        auto& e = CoreEngine::inst();
        if (e.isSessionActive(m_activeCourse)) {
            int left = e.sessionSecondsLeft(m_activeCourse);
            if (m_circTimer)   m_circTimer->setSecondsLeft(left);
            if (m_sessionStat) m_sessionStat->setText(
                    "Session active · "+m_activeCourse+" · "+QString::number(left)+"s remaining");
        } else {
            // expired
            if (m_otpLabel) {
                m_otpLabel->setText("EXPIRED");
                m_otpLabel->setObjectName("otpExpired");
                m_otpLabel->style()->unpolish(m_otpLabel); m_otpLabel->style()->polish(m_otpLabel);
            }
            if (m_sessionStat) m_sessionStat->setText("Session expired.");
            if (m_startBtn)    m_startBtn->setEnabled(true);
            if (m_stopBtn)     m_stopBtn->setEnabled(false);
            if (m_courseCombo) m_courseCombo->setEnabled(true);
            if (m_circTimer)   m_circTimer->setSecondsLeft(0);
            m_activeCourse.clear();
        }
    }

    // ── Attendance ────────────────────────────
    QWidget* buildAttendanceTab() {
        auto* w = new QWidget();
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(24,24,24,24); lay->setSpacing(16);

        lay->addWidget(makeTitle("Attendance Records"));
        lay->addWidget(makeSeparator());

        auto* row = new QHBoxLayout();
        row->addWidget(new QLabel("Course:"));
        m_attCourse = new QComboBox(); m_attCourse->setMinimumWidth(260);
        auto* refBtn = new QPushButton("Refresh");
        row->addWidget(m_attCourse); row->addWidget(refBtn); row->addStretch();
        lay->addLayout(row);

        m_attTable = new QTableWidget(0,6);
        m_attTable->setHorizontalHeaderLabels(
            {"Username","Full Name","Attended","Total","Exemptions","Attendance %"});
        m_attTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_attTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_attTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_attTable->setAlternatingRowColors(true);
        m_attTable->verticalHeader()->setVisible(false);

        connect(refBtn, &QPushButton::clicked, this, &TeacherDashboard::refreshAttendance);
        connect(m_attCourse, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &TeacherDashboard::refreshAttendance);
        lay->addWidget(m_attTable);

        // Medical exemption control: the teacher selects a student row,
        // enters the number of exempted sessions, and the existing CoreEngine
        // method performs authorization, validation, update, audit, and save.
        auto* medicalRow = new QHBoxLayout();
        auto* medicalBtn = new QPushButton("Grant Medical Exemption");
        medicalBtn->setObjectName("successBtn");
        medicalBtn->setFixedHeight(42);
        medicalRow->addWidget(medicalBtn);
        medicalRow->addStretch();
        lay->addLayout(medicalRow);

        connect(medicalBtn, &QPushButton::clicked, this, [this]() {
            if (!m_attTable || !m_attCourse || m_attCourse->currentIndex() < 0) {
                QMessageBox::warning(this, "Medical Exemption",
                                     "Please select a course first.");
                return;
            }

            const int row = m_attTable->currentRow();
            if (row < 0 || !m_attTable->item(row, 0)) {
                QMessageBox::warning(this, "Medical Exemption",
                                     "Select a student from the attendance table first.");
                return;
            }

            const QString studentUsername = m_attTable->item(row, 0)->text().trimmed();
            const QString courseCode = m_attCourse->currentData().toString();

            bool ok = false;
            const int sessions = QInputDialog::getInt(
                this,
                "Medical Exemption",
                "Number of medically exempted sessions:",
                1,
                1,
                100,
                1,
                &ok);

            if (!ok)
                return;

            auto& e = CoreEngine::inst();
            if (!e.grantMedicalExemption(studentUsername, courseCode, sessions)) {
                QMessageBox::warning(
                    this,
                    "Medical Exemption Denied",
                    "The exemption could not be granted.\n\n"
                    "Check that the student belongs to this course, "
                    "the selected course is assigned to you, and the "
                    "number of sessions is valid.");
                return;
            }

            QMessageBox::information(
                this,
                "Medical Exemption Granted",
                QString("%1 medical exemption session(s) granted to %2 for %3.")
                    .arg(sessions)
                    .arg(studentUsername)
                    .arg(courseCode));

            refreshAttendance();
        });

        return w;
    }

    void refreshAttendance() {
        if (!m_attTable || !m_attCourse) return;
        m_attTable->setRowCount(0);
        if (m_attCourse->currentIndex()<0) return;
        QString cc = m_attCourse->currentData().toString();
        auto& e = CoreEngine::inst();
        for (const auto& r : e.getCourseAttendance(cc)) {
            double pct = e.calcAttendance(r);
            int row = m_attTable->rowCount();
            m_attTable->insertRow(row);
            QString fname = e.users.contains(r.studentUsername)
                                ? e.users[r.studentUsername].fullName : r.studentUsername;
            QStringList cells = {r.studentUsername,fname,
                                 QString::number(r.attendedSessions),
                                 QString::number(r.totalSessions),
                                 QString::number(r.medicalExemptions),
                                 QString::number(pct,'f',1)+"%"};
            for (int i=0; i<cells.size(); ++i) {
                auto* item = new QTableWidgetItem(cells[i]);
                item->setTextAlignment(Qt::AlignCenter);
                if (i==5) item->setForeground(
                        pct>=ATTENDANCE_THRESHOLD?QColor("#34d399"):
                            (pct>=60.0?QColor("#fbbf24"):QColor("#f87171")));
                m_attTable->setItem(row,i,item);
            }
            m_attTable->setRowHeight(row,38);
        }
    }

    // ── My Courses ────────────────────────────
    QWidget* buildCoursesTab() {
        auto* w = new QWidget();
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(24,24,24,24); lay->setSpacing(16);

        lay->addWidget(makeTitle("My Courses"));
        lay->addWidget(makeSeparator());

        m_coursesTable = new QTableWidget(0,4);
        m_coursesTable->setHorizontalHeaderLabels({"Code","Name","Semester","Enrolled Students"});
        m_coursesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_coursesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_coursesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_coursesTable->setAlternatingRowColors(true);
        m_coursesTable->verticalHeader()->setVisible(false);
        lay->addWidget(m_coursesTable);
        lay->addStretch();
        return w;
    }

    void refreshMyCourses() {
        if (!m_coursesTable) return;
        m_coursesTable->setRowCount(0);
        auto& e = CoreEngine::inst();
        if (!e.currentUser) return;
        for (const auto& code : e.getTeacherCourses(e.currentUser->username)) {
            const auto& c = e.courses[code];
            int enrolled = e.getCourseAttendance(code).size();
            int row = m_coursesTable->rowCount();
            m_coursesTable->insertRow(row);
            QStringList cells = {c.code,c.name,c.semester,QString::number(enrolled)};
            for (int i=0; i<cells.size(); ++i) {
                auto* item = new QTableWidgetItem(cells[i]);
                item->setTextAlignment(Qt::AlignCenter);
                m_coursesTable->setItem(row,i,item);
            }
            m_coursesTable->setRowHeight(row,38);
        }
    }
};

// ─────────────────────────────────────────────
//  STUDENT DASHBOARD
// ─────────────────────────────────────────────
class StudentDashboard : public QWidget {
public:
    explicit StudentDashboard(QWidget* parent=nullptr) : QWidget(parent) {
        auto* lay = new QVBoxLayout(this);
        lay->setContentsMargins(0,0,0,0);
        m_tabs = new QTabWidget(this);
        m_tabs->addTab(buildOtpTab(),        "  Submit Attendance  ");
        m_tabs->addTab(buildAttendanceTab(), "  My Attendance  ");
        m_tabs->addTab(buildAdmitTab(),      "  Admit Cards  ");
        connect(m_tabs,&QTabWidget::currentChanged,this,[this](int idx){
            if (idx==1) refreshAttendance();
            if (idx==2) refreshAdmitCards();
        });
        lay->addWidget(m_tabs);
    }
    void setNavigationHandler(NavigationHandler* handler) {
        m_navigation = handler;
    }

    void onActivated() { refreshCourseCombo(); refreshAttendance(); refreshAdmitCards(); }

private:
    NavigationHandler* m_navigation = nullptr;
    QTabWidget*  m_tabs        = nullptr;
    QComboBox*   m_courseCombo = nullptr;
    QLineEdit*   m_otpEdit     = nullptr;
    QLabel*      m_submitStatus= nullptr;
    QScrollArea* m_attScroll   = nullptr;
    QWidget*     m_attContent  = nullptr;
    QScrollArea* m_admitScroll = nullptr;
    QWidget*     m_admitContent= nullptr;

    QWidget* buildOtpTab() {
        auto* w = new QWidget();
        auto* outer = new QVBoxLayout(w);
        outer->setContentsMargins(0,0,0,0);
        outer->addStretch();

        auto* row = new QHBoxLayout();
        row->addStretch();
        auto* card = new QFrame(); card->setObjectName("card"); card->setFixedWidth(440);
        auto* lay  = new QVBoxLayout(card);
        lay->setContentsMargins(32,32,32,32); lay->setSpacing(18);

        auto* title = new QLabel("Submit Attendance"); title->setObjectName("titleLabel");
        title->setAlignment(Qt::AlignCenter);
        auto* sub = new QLabel("Enter the OTP provided by your teacher.");
        sub->setObjectName("subtitleLabel"); sub->setAlignment(Qt::AlignCenter);
        sub->setWordWrap(true);

        m_courseCombo = new QComboBox(); m_courseCombo->setMinimumHeight(44);
        m_otpEdit = new QLineEdit(); m_otpEdit->setObjectName("otpInput");
        m_otpEdit->setPlaceholderText("4-CHARACTER OTP"); m_otpEdit->setMaxLength(OTP_LENGTH);
        m_otpEdit->setAlignment(Qt::AlignCenter); m_otpEdit->setMinimumHeight(60);

        auto* submitBtn = new QPushButton("Mark Attendance");
        submitBtn->setObjectName("successBtn"); submitBtn->setMinimumHeight(44);

        m_submitStatus = new QLabel(""); m_submitStatus->setAlignment(Qt::AlignCenter);
        m_submitStatus->setWordWrap(true);

        auto* logoutBtn = new QPushButton("Logout"); logoutBtn->setObjectName("logoutBtn");
        logoutBtn->setFixedWidth(100);

        connect(submitBtn, &QPushButton::clicked, this, &StudentDashboard::onSubmit);
        connect(m_otpEdit, &QLineEdit::returnPressed, this, &StudentDashboard::onSubmit);
        connect(logoutBtn, &QPushButton::clicked, this,[this]{
            CoreEngine::inst().logout(); if (m_navigation) m_navigation->handleLogout();
        });

        lay->addWidget(title); lay->addWidget(sub);
        lay->addWidget(new QLabel("Course:")); lay->addWidget(m_courseCombo);
        lay->addWidget(new QLabel("OTP Code:")); lay->addWidget(m_otpEdit);
        lay->addWidget(submitBtn); lay->addWidget(m_submitStatus);
        lay->addWidget(logoutBtn, 0, Qt::AlignRight);

        row->addWidget(card); row->addStretch();
        outer->addLayout(row); outer->addStretch();
        return w;
    }

    void refreshCourseCombo() {
        if (!m_courseCombo) return;
        m_courseCombo->clear();
        for (const auto& c : CoreEngine::inst().courses)
            if (c.isActive) m_courseCombo->addItem(c.code+" — "+c.name, c.code);
    }

    void onSubmit() {
        auto& e = CoreEngine::inst();
        if (!m_courseCombo || m_courseCombo->currentIndex()<0) {
            m_submitStatus->setStyleSheet("color:#f87171;");
            m_submitStatus->setText("Please select a course."); return;
        }
        QString cc  = m_courseCombo->currentData().toString();
        QString otp = m_otpEdit->text().trimmed();
        if (otp.isEmpty()) {
            m_submitStatus->setStyleSheet("color:#f87171;");
            m_submitStatus->setText("Please enter the OTP."); return;
        }
        if (e.submitAttendance(cc, otp)) {
            m_submitStatus->setStyleSheet("color:#34d399;font-weight:600;");
            m_submitStatus->setText("✓  Attendance recorded for "+cc+"!");
            m_otpEdit->clear();
        } else {
            m_submitStatus->setStyleSheet("color:#f87171;font-weight:600;");
            m_submitStatus->setText(
                !e.isSessionActive(cc)
                    ? "✗  No active session for "+cc+" or session has expired."
                    : "✗  Invalid OTP or attendance already marked.");
        }
    }

    // ── My Attendance ─────────────────────────
    QWidget* buildAttendanceTab() {
        auto* w = new QWidget();
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(24,24,24,24); lay->setSpacing(16);
        lay->addWidget(makeTitle("My Attendance")); lay->addWidget(makeSeparator());
        m_attScroll = new QScrollArea(); m_attScroll->setWidgetResizable(true);
        m_attContent = new QWidget(); m_attScroll->setWidget(m_attContent);
        new QVBoxLayout(m_attContent);
        lay->addWidget(m_attScroll);
        return w;
    }

    void refreshAttendance() {
        if (!m_attScroll) return;
        auto& e = CoreEngine::inst();
        if (!e.currentUser) return;

        QWidget* newContent = new QWidget();
        auto* lay = new QVBoxLayout(newContent);
        lay->setContentsMargins(8,8,8,8); lay->setSpacing(16);

        auto records = e.getStudentAttendance(e.currentUser->username);
        if (records.isEmpty()) {
            auto* msg = new QLabel("No attendance records found.");
            msg->setStyleSheet("color:#64748b;font-size:14px;"); msg->setAlignment(Qt::AlignCenter);
            lay->addWidget(msg);
        } else {
            for (const auto& r : records) {
                double pct = e.calcAttendance(r);
                QString cname = e.courses.contains(r.courseCode)
                                    ? e.courses[r.courseCode].name : r.courseCode;

                auto* card = new QFrame(); card->setObjectName("card");
                auto* cl   = new QVBoxLayout(card);
                cl->setContentsMargins(20,16,20,16); cl->setSpacing(10);

                auto* row1 = new QHBoxLayout();
                auto* codeL = new QLabel(r.courseCode+"  ·  "+cname);
                codeL->setStyleSheet("font-weight:600;font-size:14px;color:#f1f5f9;");
                auto* badge = new QLabel(StyleManager::getAttendanceText(pct));
                badge->setObjectName(StyleManager::getAttendanceBadgeId(pct));
                row1->addWidget(codeL); row1->addStretch(); row1->addWidget(badge);

                auto* prog = new QProgressBar();
                QString pid = StyleManager::getProgressBarId(pct);
                if (!pid.isEmpty()) prog->setObjectName(pid);
                prog->setRange(0,100); prog->setValue(static_cast<int>(pct));
                prog->setFormat(QString::number(pct,'f',1)+"%  ("+
                                QString::number(r.attendedSessions)+"/"+
                                QString::number(r.totalSessions)+")");
                prog->setFixedHeight(22);

                auto* sr = new QHBoxLayout();
                int needed = qMax(0, static_cast<int>(std::ceil(
                                         0.75*r.totalSessions - r.attendedSessions - r.medicalExemptions)));
                auto addStat = [&](const QString& lbl, const QString& val) {
                    auto* col = new QVBoxLayout();
                    auto* vl  = new QLabel(val);
                    vl->setStyleSheet("font-size:20px;font-weight:700;color:#38bdf8;");
                    auto* ll  = new QLabel(lbl);
                    ll->setStyleSheet("font-size:10px;color:#64748b;");
                    col->addWidget(vl,0,Qt::AlignCenter);
                    col->addWidget(ll,0,Qt::AlignCenter);
                    sr->addLayout(col);
                };
                addStat("Attended",   QString::number(r.attendedSessions));
                addStat("Total",      QString::number(r.totalSessions));
                addStat("Exemptions", QString::number(r.medicalExemptions));
                addStat("Still Need", QString::number(needed));

                cl->addLayout(row1);
                cl->addWidget(prog);
                cl->addLayout(sr);
                lay->addWidget(card);
            }
        }
        lay->addStretch();
        m_attScroll->setWidget(newContent);
        // old content auto-deleted as it had no parent after setWidget
        m_attContent = newContent;
    }

    // ── Admit Cards ───────────────────────────
    QWidget* buildAdmitTab() {
        auto* w = new QWidget();
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(24,24,24,24); lay->setSpacing(16);
        lay->addWidget(makeTitle("My Admit Cards")); lay->addWidget(makeSeparator());
        m_admitScroll = new QScrollArea(); m_admitScroll->setWidgetResizable(true);
        m_admitContent = new QWidget(); m_admitScroll->setWidget(m_admitContent);
        new QVBoxLayout(m_admitContent);
        lay->addWidget(m_admitScroll);
        return w;
    }

    void refreshAdmitCards() {
        if (!m_admitScroll) return;
        auto& e = CoreEngine::inst();
        if (!e.currentUser) return;

        QWidget* newContent = new QWidget();
        auto* lay = new QVBoxLayout(newContent);
        lay->setContentsMargins(8,8,8,8); lay->setSpacing(16);

        auto cards = e.getStudentAdmitCards(e.currentUser->username);
        if (cards.isEmpty()) {
            auto* msg = new QLabel(
                "No admit cards yet.\nCards are generated by the administrator.");
            msg->setStyleSheet("color:#64748b;font-size:14px;"); msg->setAlignment(Qt::AlignCenter);
            msg->setWordWrap(true);
            lay->addWidget(msg);
        } else {
            for (const auto& card : cards) {
                auto* frame = new QFrame(); frame->setObjectName("card");
                QString borderColor = card.approved ? "#34d399" : "#f87171";
                frame->setStyleSheet("QFrame#card{border:1.5px solid "+borderColor+";}");
                auto* fl = new QVBoxLayout(frame);
                fl->setContentsMargins(24,20,24,20); fl->setSpacing(12);

                auto* hdr = new QHBoxLayout();
                auto* logo = new QLabel("GEHU");
                logo->setStyleSheet("font-size:20px;font-weight:900;color:#60a5fa;letter-spacing:3px;");
                auto* hdrT = new QLabel("Admit Card · "+card.courseCode);
                hdrT->setStyleSheet("font-size:15px;font-weight:600;color:#f1f5f9;");
                auto* stat = new QLabel(card.approved?"✓ ELIGIBLE":"✗ BLOCKED");
                stat->setObjectName(card.approved?"badge_ok":"badge_err");
                hdr->addWidget(logo); hdr->addWidget(hdrT); hdr->addStretch(); hdr->addWidget(stat);
                fl->addLayout(hdr);
                fl->addWidget(makeSeparator());

                auto* grid = new QGridLayout(); grid->setSpacing(10);
                auto addF = [&](int row, int col, const QString& lbl, const QString& val){
                    auto* l = new QLabel(lbl);
                    l->setStyleSheet("font-size:10px;color:#64748b;text-transform:uppercase;");
                    auto* v = new QLabel(val);
                    v->setStyleSheet("font-size:14px;font-weight:600;color:#f1f5f9;");
                    grid->addWidget(l,row*2,col);
                    grid->addWidget(v,row*2+1,col);
                };
                addF(0,0,"Student",   card.studentFullName);
                addF(0,1,"Username",  card.studentUsername);
                addF(1,0,"Course",    card.courseCode);
                addF(1,1,"Exam Date", card.examDate.isEmpty()?"TBD":card.examDate);
                addF(2,0,"Seat No",   card.seatNumber);
                addF(2,1,"Generated", card.generatedAt);
                fl->addLayout(grid);

                auto* sig = new QLabel("SHA-256: "+card.signature);
                sig->setStyleSheet(
                    "font-family:monospace;font-size:10px;color:#334155;padding:6px;");
                sig->setWordWrap(true);
                fl->addWidget(sig);
                lay->addWidget(frame);
            }
        }
        lay->addStretch();
        m_admitScroll->setWidget(newContent);
        m_admitContent = newContent;
    }
};

// ─────────────────────────────────────────────
//  LOGIN WIDGET
// ─────────────────────────────────────────────
class LoginWidget : public QWidget {
public:
    explicit LoginWidget(QWidget* parent=nullptr) : QWidget(parent) {
        setStyleSheet("background-color:#0a0d16;");
        auto* outer = new QVBoxLayout(this);
        outer->setContentsMargins(0,0,0,0);
        outer->addStretch(2);

        auto* row = new QHBoxLayout();
        row->addStretch();

        auto* card = new QFrame();
        card->setObjectName("card"); card->setFixedWidth(420);
        card->setStyleSheet(
            "QFrame#card{background-color:#13161f;"
            "border:1px solid #1e2640;border-radius:16px;}");
        auto* lay = new QVBoxLayout(card);
        lay->setContentsMargins(40,40,40,40); lay->setSpacing(16);

        auto* brand = new QLabel("GEHU");
        brand->setStyleSheet(
            "font-size:38px;font-weight:900;color:#3b82f6;letter-spacing:6px;");
        brand->setAlignment(Qt::AlignCenter);

        auto* sub = new QLabel("Attendance Management System");
        sub->setObjectName("subtitleLabel"); sub->setAlignment(Qt::AlignCenter);

        auto* ver = new QLabel("v"+APP_VERSION);
        ver->setStyleSheet("color:#334155;font-size:11px;");
        ver->setAlignment(Qt::AlignCenter);

        m_usernameEdit = new QLineEdit();
        m_usernameEdit->setPlaceholderText("Username");
        m_usernameEdit->setMinimumHeight(44);

        m_passwordEdit = new QLineEdit();
        m_passwordEdit->setPlaceholderText("Password");
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_passwordEdit->setMinimumHeight(44);

        auto* loginBtn = new QPushButton("Sign In");
        loginBtn->setMinimumHeight(44);
        loginBtn->setStyleSheet(
            "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "stop:0 #2563eb,stop:1 #38bdf8);color:#fff;border-radius:8px;"
            "font-size:14px;font-weight:700;}"
            "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "stop:0 #3b82f6,stop:1 #60c8ff);}"
            "QPushButton:pressed{background:#1d4ed8;}");

        m_errorLabel = new QLabel(""); m_errorLabel->setAlignment(Qt::AlignCenter);
        m_errorLabel->setStyleSheet("color:#f87171;font-size:12px;");
        m_errorLabel->setWordWrap(true);

        // Demo hints
        auto* hint = new QFrame();
        hint->setStyleSheet("background:#0f1a35;border-radius:8px;");
        auto* hl = new QVBoxLayout(hint);
        hl->setContentsMargins(12,10,12,10); hl->setSpacing(3);
        auto* ht = new QLabel("Demo Credentials");
        ht->setStyleSheet("color:#38bdf8;font-size:11px;font-weight:700;");
        auto* hb = new QLabel(
            "Admin:   admin / admin123\n"
            "Teacher: drsingh / teacher123\n"
            "Student: stu001 / student123");
        hb->setStyleSheet("color:#64748b;font-size:11px;");
        hl->addWidget(ht); hl->addWidget(hb);

        connect(loginBtn,     &QPushButton::clicked,       this, &LoginWidget::doLogin);
        connect(m_passwordEdit,&QLineEdit::returnPressed,  this, &LoginWidget::doLogin);
        connect(m_usernameEdit,&QLineEdit::returnPressed,  this,[this]{m_passwordEdit->setFocus();});

        lay->addWidget(brand); lay->addWidget(sub); lay->addWidget(ver);
        lay->addWidget(makeSeparator()); lay->addSpacing(4);
        lay->addWidget(new QLabel("Username:")); lay->addWidget(m_usernameEdit);
        lay->addWidget(new QLabel("Password:")); lay->addWidget(m_passwordEdit);
        lay->addWidget(loginBtn); lay->addWidget(m_errorLabel); lay->addWidget(hint);

        row->addWidget(card); row->addStretch();
        outer->addLayout(row);
        outer->addStretch(2);

        auto* footer = new QLabel("GEHU · Attendance Management System · v"+APP_VERSION);
        footer->setStyleSheet("color:#1e2640;font-size:11px;");
        footer->setAlignment(Qt::AlignCenter);
        outer->addWidget(footer);
    }

    void setNavigationHandler(NavigationHandler* handler) {
        m_navigation = handler;
    }

    void clearForm() {
        m_usernameEdit->clear(); m_passwordEdit->clear(); m_errorLabel->clear();
        m_usernameEdit->setFocus();
    }


private:
    NavigationHandler* m_navigation = nullptr;
    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QLabel*    m_errorLabel   = nullptr;

    void doLogin() {
        m_errorLabel->clear();
        QString user = m_usernameEdit->text().trimmed();
        QString pass = m_passwordEdit->text();
        if (user.isEmpty() || pass.isEmpty()) {
            m_errorLabel->setText("Please fill in both fields."); return;
        }
        if (CoreEngine::inst().login(user, pass)) {
            m_passwordEdit->clear();
            if (m_navigation) m_navigation->handleLogin(CoreEngine::inst().currentUser->role);
        } else {
            m_passwordEdit->clear();
            m_errorLabel->setText("Invalid username or password.");
        }
    }
};

// ─────────────────────────────────────────────
//  MAIN WINDOW
// ─────────────────────────────────────────────
class MainWindow : public QWidget, public NavigationHandler {
public:
    explicit MainWindow(QWidget* parent=nullptr) : QWidget(parent) {
        setWindowTitle("Attendance Management System  |  GEHU");
        resize(1100,720);
        setMinimumSize(860,580);

        m_stack   = new QStackedWidget(this);
        m_login   = new LoginWidget(this);
        m_admin   = new AdminDashboard(this);
        m_teacher = new TeacherDashboard(this);
        m_student = new StudentDashboard(this);

        m_stack->addWidget(m_login);
        m_stack->addWidget(m_admin);
        m_stack->addWidget(m_teacher);
        m_stack->addWidget(m_student);

        auto* lay = new QVBoxLayout(this);
        lay->setContentsMargins(0,0,0,0);
        lay->addWidget(m_stack);

        m_login->setNavigationHandler(this);
        m_admin->setNavigationHandler(this);
        m_teacher->setNavigationHandler(this);
        m_student->setNavigationHandler(this);

        m_stack->setCurrentWidget(m_login);
    }

private:
    QStackedWidget*   m_stack   = nullptr;
    LoginWidget*      m_login   = nullptr;
    AdminDashboard*   m_admin   = nullptr;
    TeacherDashboard* m_teacher = nullptr;
    StudentDashboard* m_student = nullptr;

    void handleLogin(Role role) override {
        if      (role==Role::Admin)   { m_admin->onActivated();   m_stack->setCurrentWidget(m_admin);   }
        else if (role==Role::Teacher) { m_teacher->onActivated(); m_stack->setCurrentWidget(m_teacher); }
        else if (role==Role::Student) { m_student->onActivated(); m_stack->setCurrentWidget(m_student); }
        else handleLogout();
    }
    void handleLogout() override { m_login->clearForm(); m_stack->setCurrentWidget(m_login); }
};

// ─────────────────────────────────────────────
//  ENTRY POINT
// ─────────────────────────────────────────────
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Attendance Management System");
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName("GEHU");

    app.setStyleSheet(StyleManager::getStylesheet());
    CoreEngine::inst().load();

    MainWindow w;
    w.show();
    return app.exec();
}
