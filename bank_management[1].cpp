#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <limits>

using namespace std;

struct Transaction {
    time_t t;
    string detail;
    Transaction(const string &d) : t(time(nullptr)), detail(d) {}
    string toString() const {
        tm *lt = localtime(&t);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
        return string(buf) + "  " + detail;
    }
};

class Account {
protected:
    string name;
    long long accNo;
    float balance;
    int pin;
    bool blocked;
    vector<Transaction> history;
    time_t createdAt;
    time_t lastTxn;
    static long long nextAccNo;
public:
    Account() : name(""), accNo(0), balance(0), pin(0), blocked(false), createdAt(time(nullptr)), lastTxn(0) {}
    virtual ~Account() {}
    void initNew(const string &n, int p) {
        name = n;
        pin = p;
        accNo = nextAccNo++;
        balance = 0;
        createdAt = time(nullptr);
        lastTxn = 0;
        history.clear();
    }
    long long getAccNo() const { return accNo; }
    string getName() const { return name; }
    float getBalance() const { return balance; }
    bool isBlocked() const { return blocked; }
    bool checkPin(int p) const { return pin == p; }
    void block() { blocked = true; }
    void unblock() { blocked = false; }
    void changePin(int newPin) { pin = newPin; }
    void addHistory(const string &d) { history.emplace_back(d); lastTxn = time(nullptr); }
    vector<Transaction> getHistory() const { return history; }
    time_t getCreatedAt() const { return createdAt; }
    time_t getLastTxn() const { return lastTxn; }
    virtual void deposit(float amt) {
        balance += amt;
        addHistory("+ " + to_string(amt) + " Deposit");
    }
    virtual bool withdraw(float amt) {
        if (balance >= amt) {
            balance -= amt;
            addHistory("- " + to_string(amt) + " Withdraw");
            return true;
        }
        return false;
    }
    virtual string accType() const = 0;
    virtual void display() const {
        cout << left << setw(18) << "Name:" << name << "\n";
        cout << left << setw(18) << "Account No:" << accNo << "\n";
        cout << left << setw(18) << "Type:" << accType() << "\n";
        cout << left << setw(18) << "Balance:" << balance << "\n";
        tm *ct = localtime(&createdAt);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d", ct);
        cout << left << setw(18) << "Created On:" << buf << "\n";
        if (lastTxn) {
            tm *lt = localtime(&lastTxn);
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
            cout << left << setw(18) << "Last Transaction:" << buf << "\n";
        } else {
            cout << left << setw(18) << "Last Transaction:" << "No transactions\n";
        }
    }
};

long long Account::nextAccNo = 200001;

class SavingsAccount : public Account {
public:
    string accType() const override { return "Savings"; }
    void deposit(float amt) override {
        balance += amt;
        float interest = (balance * 2) / 100.0f;
        balance += interest;
        addHistory("+ " + to_string(amt) + " Deposit (interest " + to_string(interest) + ")");
    }
    bool withdraw(float amt) override {
        if (balance > amt) {
            balance -= amt;
            addHistory("- " + to_string(amt) + " Withdraw");
            return true;
        }
        return false;
    }
};

class CurrentAccount : public Account {
public:
    string accType() const override { return "Current"; }
    bool withdraw(float amt) override {
        if (balance - amt >= 1000.0f) {
            balance -= amt;
            addHistory("- " + to_string(amt) + " Withdraw");
            return true;
        }
        return false;
    }
};

class Node {
public:
    Account* acc;
    Node* next;
    Node(Account* a) : acc(a), next(nullptr) {}
};

class Bank {
private:
    Node* head;
public:
    Bank() : head(nullptr) {}
    ~Bank() {
        Node* cur = head;
        while (cur) {
            Node* t = cur;
            cur = cur->next;
            delete t->acc;
            delete t;
        }
    }
    void addAccount(Account* a) {
        Node* n = new Node(a);
        n->next = head;
        head = n;
    }
    Account* search(long long accNo) const {
        Node* t = head;
        while (t) {
            if (t->acc->getAccNo() == accNo) return t->acc;
            t = t->next;
        }
        return nullptr;
    }
    bool deleteAccount(long long accNo) {
        Node* t = head; Node* prev = nullptr;
        while (t && t->acc->getAccNo() != accNo) { prev = t; t = t->next; }
        if (!t) return false;
        if (!prev) head = t->next; else prev->next = t->next;
        delete t->acc; delete t;
        return true;
    }
    vector<Account*> toVector() const {
        vector<Account*> v;
        Node* t = head;
        while (t) { v.push_back(t->acc); t = t->next; }
        return v;
    }
    void rebuildFromVector(const vector<Account*> &v) {
        Node* cur = head;
        while (cur) { Node* n = cur; cur = cur->next; delete n; }
        head = nullptr;
        for (Account* a : v) {
            Node* n = new Node(a);
            n->next = head;
            head = n;
        }
    }
    void sortByName() {
        vector<Account*> v = toVector();
        sort(v.begin(), v.end(), [](Account* a, Account* b){ return a->getName() < b->getName(); });
        rebuildFromVector(v);
    }
    void sortByBalanceDesc() {
        vector<Account*> v = toVector();
        sort(v.begin(), v.end(), [](Account* a, Account* b){ return a->getBalance() > b->getBalance(); });
        rebuildFromVector(v);
    }
    void sortByAccNo() {
        vector<Account*> v = toVector();
        sort(v.begin(), v.end(), [](Account* a, Account* b){ return a->getAccNo() < b->getAccNo(); });
        rebuildFromVector(v);
    }
    void displayAll() const {
        Node* t = head;
        if (!t) { cout << "\nNo accounts in the bank.\n"; return; }
        while (t) {
            cout << "-------------------------------\n";
            t->acc->display();
            t = t->next;
        }
    }
    float totalBankBalance() const {
        float s = 0;
        Node* t = head;
        while (t) { s += t->acc->getBalance(); t = t->next; }
        return s;
    }
    vector<Account*> accountsList() const { return toVector(); }
};

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readIntSafe() {
    int x;
    while (!(cin >> x)) {
        clearInput();
        cout << "Invalid input. Enter number: ";
    }
    clearInput();
    return x;
}

long long readLongSafe() {
    long long x;
    while (!(cin >> x)) {
        clearInput();
        cout << "Invalid input. Enter number: ";
    }
    clearInput();
    return x;
}

int askPinInput() {
    int p;
    while (!(cin >> p)) {
        clearInput();
        cout << "Enter 4-digit PIN: ";
    }
    clearInput();
    return p;
}

void showMiniStatement(Account* a, int n = 10) {
    vector<Transaction> h = a->getHistory();
    int sz = h.size();
    int start = max(0, sz - n);
    if (sz == 0) {
        cout << "\nNo transactions yet.\n";
        return;
    }
    cout << "\n---- Mini Statement (last " << (sz - start) << ") ----\n";
    for (int i = start; i < sz; ++i) cout << h[i].toString() << "\n";
    cout << "--------------------------------\n";
}

bool authenticateAccount(Account* a) {
    if (!a) return false;
    if (a->isBlocked()) { cout << "\nAccount is blocked.\n"; return false; }
    int attempts = 0;
    while (attempts < 3) {
        cout << "Enter 4-digit PIN: ";
        int p = askPinInput();
        if (a->checkPin(p)) return true;
        attempts++;
        cout << "Wrong PIN. Attempts left: " << (3 - attempts) << "\n";
    }
    a->block();
    cout << "Account blocked due to 3 failed PIN attempts.\n";
    return false;
}

void userATMMenu(Bank &bank, Account* a) {
    while (true) {
        cout << "\n---- ATM MENU ----\n";
        cout << "1 -> Check Balance\n2 -> Deposit\n3 -> Withdraw\n4 -> Mini Statement\n5 -> Transfer Funds\n6 -> Change PIN\n7 -> Delete Account\n8 -> Logout\nEnter choice: ";
        int ch = readIntSafe();
        if (ch == 1) {
            if (!authenticateAccount(a)) continue;
            cout << "\nBalance: " << a->getBalance() << "\n";
        } else if (ch == 2) {
            if (!authenticateAccount(a)) continue;
            cout << "Enter amount to deposit: ";
            float amt; while (!(cin >> amt)) { clearInput(); cout << "Enter number: "; }
            clearInput();
            if (amt > 0) { a->deposit(amt); cout << "Deposit successful.\n"; } else cout << "Invalid amount.\n";
        } else if (ch == 3) {
            if (!authenticateAccount(a)) continue;
            cout << "Enter amount to withdraw: ";
            float amt; while (!(cin >> amt)) { clearInput(); cout << "Enter number: "; }
            clearInput();
            if (amt > 0) {
                if (a->withdraw(amt)) cout << "Withdraw successful.\n";
                else cout << "Insufficient balance or min balance requirement.\n";
            } else cout << "Invalid amount.\n";
        } else if (ch == 4) {
            if (!authenticateAccount(a)) continue;
            showMiniStatement(a, 10);
        } else if (ch == 5) {
            if (!authenticateAccount(a)) continue;
            cout << "Enter receiver account number: ";
            long long r = readLongSafe();
            Account* recv = bank.search(r);
            if (!recv) { cout << "Receiver account not found.\n"; continue; }
            cout << "Enter amount to transfer: ";
            float amt; while (!(cin >> amt)) { clearInput(); cout << "Enter number: "; }
            clearInput();
            if (amt > 0 && a->withdraw(amt)) {
                recv->deposit(amt);
                a->addHistory("- " + to_string(amt) + " Transfer to " + to_string(r));
                recv->addHistory("+ " + to_string(amt) + " Transfer from " + to_string(a->getAccNo()));
                cout << "Transfer successful.\n";
            } else cout << "Transfer failed (insufficient balance or invalid amount).\n";
        } else if (ch == 6) {
            if (!authenticateAccount(a)) continue;
            cout << "Enter new 4-digit PIN: ";
            int np = askPinInput();
            a->changePin(np);
            cout << "PIN changed successfully.\n";
        } else if (ch == 7) {
            if (!authenticateAccount(a)) continue;
            cout << "Type YES to confirm deletion of account " << a->getAccNo() << " : ";
            string confirm; getline(cin, confirm);
            if (confirm == "YES") {
                long long acc = a->getAccNo();
                if (bank.deleteAccount(acc)) { cout << "Account deleted.\n"; return; }
                else cout << "Delete failed.\n";
            } else cout << "Deletion cancelled.\n";
        } else if (ch == 8) {
            cout << "Logging out.\n";
            break;
        } else {
            cout << "Invalid choice.\n";
        }
    }
}

void adminMenu(Bank &bank) {
    const string adminUser = "admin";
    const string adminPass = "admin123";
    cout << "Admin Username: ";
    string u; getline(cin, u);
    cout << "Admin Password: ";
    string p; getline(cin, p);
    if (u != adminUser || p != adminPass) { cout << "Invalid admin credentials.\n"; return; }
    while (true) {
        cout << "\n---- ADMIN MENU ----\n";
        cout << "1 -> View All Accounts\n2 -> Search Account\n3 -> Delete Account\n4 -> Edit Customer Details\n5 -> Total Bank Balance\n6 -> Sort Accounts by Name\n7 -> Sort Accounts by Balance (Desc)\n8 -> Sort Accounts by Account No\n9 -> Back\nEnter choice: ";
        int ch = readIntSafe();
        if (ch == 1) bank.displayAll();
        else if (ch == 2) {
            cout << "Enter acc no: ";
            long long acc = readLongSafe();
            Account* a = bank.search(acc);
            if (a) a->display(); else cout << "Not found.\n";
        } else if (ch == 3) {
            cout << "Enter acc no to delete: ";
            long long acc = readLongSafe();
            if (bank.deleteAccount(acc)) cout << "Deleted.\n"; else cout << "Not found.\n";
        } else if (ch == 4) {
            cout << "Enter acc no to edit: ";
            long long acc = readLongSafe();
            Account* a = bank.search(acc);
            if (!a) { cout << "Not found.\n"; continue; }
            cout << "1 -> Edit Name\n2 -> Change PIN\nEnter choice: ";
            int e = readIntSafe();
            if (e == 1) {
                cout << "Enter new name: ";
                string nm; getline(cin, nm);
                a->initNew(nm, 0);
                cout << "Name updated. NOTE: PIN reset to 000 (set new PIN via ATM).\n";
            } else if (e == 2) {
                cout << "Enter new 4-digit PIN: ";
                int np = askPinInput();
                a->changePin(np);
                cout << "PIN changed.\n";
            } else cout << "Invalid option.\n";
        } else if (ch == 5) {
            cout << "Total money in bank: " << bank.totalBankBalance() << "\n";
        } else if (ch == 6) {
            bank.sortByName();
            cout << "Sorted by name.\n";
        } else if (ch == 7) {
            bank.sortByBalanceDesc();
            cout << "Sorted by balance descending.\n";
        } else if (ch == 8) {
            bank.sortByAccNo();
            cout << "Sorted by account number.\n";
        } else if (ch == 9) break;
        else cout << "Invalid choice.\n";
    }
}

int main() {
    Bank bank;
    while (true) {
        cout << "\n==== BANK MANAGEMENT SYSTEM ====\n";
        cout << "1 -> Create Savings Account\n2 -> Create Current Account\n3 -> Login to ATM (by Account No)\n4 -> Admin Login\n5 -> Display All Accounts (brief)\n6 -> Exit\nEnter choice: ";
        int ch = readIntSafe();
        if (ch == 1 || ch == 2) {
            cout << "Enter customer name: ";
            string name; getline(cin, name);
            int p;
            while (true) {
                cout << "Set 4-digit PIN: ";
                p = askPinInput();
                if (p >= 1000 && p <= 9999) break;
                cout << "PIN must be 4 digits.\n";
            }
            Account* a = (ch == 1) ? static_cast<Account*>(new SavingsAccount()) : static_cast<Account*>(new CurrentAccount());
            a->initNew(name, p);
            bank.addAccount(a);
            cout << "Account created. Account No: " << a->getAccNo() << "\n";
        } else if (ch == 3) {
            cout << "Enter account number: ";
            long long acc = readLongSafe();
            Account* a = bank.search(acc);
            if (!a) { cout << "Account not found.\n"; continue; }
            userATMMenu(bank, a);
        } else if (ch == 4) {
            adminMenu(bank);
        } else if (ch == 5) {
            vector<Account*> v = bank.accountsList();
            if (v.empty()) cout << "No accounts.\n";
            else {
                cout << "\nAccNo\t\tName\t\tBalance\n";
                for (Account* a : v) cout << a->getAccNo() << "\t" << a->getName() << "\t\t" << a->getBalance() << "\n";
            }
        } else if (ch == 6) {
            cout << "Exiting. Goodbye.\n";
            break;
        } else {
            cout << "Invalid choice.\n";
        }
    }
    return 0;
}
