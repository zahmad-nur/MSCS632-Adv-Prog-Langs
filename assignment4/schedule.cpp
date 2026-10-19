/*
 * Employee Shift Scheduler (C++)
 * MSCS 632 - Advanced Programming Languages
 *
 * Rules:
 *   - 7 days, 3 shifts per day (Morning, Afternoon, Evening)
 *   - No employee works more than one shift per day
 *   - No employee works more than 5 days per week
 *   - At least 2 employees per shift; if short, randomly assign
 *     employees who have not yet worked 5 days
 *   - Shift capacity defaults to 5 (user-set in manual mode)
 *   - If a preferred shift is full (capacity reached), the employee is moved to
 *     another shift that day, or carried to the next day if the whole day is full
 *   - Preferences are ranked (bonus): earlier in the list = higher priority
 *
 * Build: g++ -std=c++17 -o schedule schedule.cpp
 */

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const vector<string> DAYS = {"Monday", "Tuesday", "Wednesday", "Thursday",
                             "Friday", "Saturday", "Sunday"};
const vector<string> SHIFTS = {"Morning", "Afternoon", "Evening"};
const int MIN_PER_SHIFT = 2;
const int MAX_PER_SHIFT = 5;  // default shift capacity; the prompt requires shifts to be able to fill up but sets no limit
const int MAX_DAYS = 5;

struct Employee {
    string name;
    map<string, vector<string>> prefs;  // day -> ranked shifts
};

// schedule[day][shift] -> list of employee names
using Schedule = map<string, map<string, vector<string>>>;

vector<Employee> sampleData() {
    return {
        {"Alice",  {{"Monday", {"Morning", "Afternoon"}}, {"Tuesday", {"Morning"}}, {"Wednesday", {"Morning", "Evening"}}, {"Friday", {"Morning"}}}},
        {"Bob",    {{"Monday", {"Morning", "Evening"}}, {"Tuesday", {"Afternoon", "Evening"}}, {"Thursday", {"Afternoon"}}, {"Saturday", {"Afternoon", "Morning"}}}},
        {"Carmen", {{"Monday", {"Evening"}}, {"Wednesday", {"Evening"}}, {"Thursday", {"Evening", "Afternoon"}}, {"Sunday", {"Evening"}}}},
        {"David",  {{"Monday", {"Morning"}}, {"Tuesday", {"Morning", "Afternoon"}}, {"Thursday", {"Morning"}}, {"Saturday", {"Morning"}}}},
        {"Esha",   {{"Monday", {"Morning", "Afternoon"}}, {"Tuesday", {"Evening"}}, {"Friday", {"Afternoon", "Morning"}}, {"Sunday", {"Morning", "Evening"}}}},
        {"Farid",  {{"Monday", {"Morning", "Afternoon"}}, {"Wednesday", {"Afternoon"}}, {"Friday", {"Evening"}}, {"Sunday", {"Afternoon"}}}},
        {"Grace",  {{"Tuesday", {"Afternoon", "Morning"}}, {"Wednesday", {"Afternoon", "Morning"}}, {"Friday", {"Morning"}}, {"Saturday", {"Evening"}}}},
        {"Hiro",   {{"Tuesday", {"Evening", "Afternoon"}}, {"Thursday", {"Evening"}}, {"Saturday", {"Morning"}}, {"Sunday", {"Evening"}}}},
        {"Isabel", {{"Wednesday", {"Afternoon"}}, {"Thursday", {"Afternoon"}}, {"Saturday", {"Evening", "Afternoon"}}, {"Sunday", {"Morning"}}}},
        {"Jamal",  {{"Wednesday", {"Morning"}}, {"Thursday", {"Morning", "Evening"}}, {"Friday", {"Evening", "Afternoon"}}, {"Sunday", {"Afternoon", "Morning"}}}},
        {"Kenji",  {{"Monday", {"Morning", "Evening"}}, {"Tuesday", {"Morning"}}, {"Friday", {"Afternoon"}}, {"Saturday", {"Afternoon"}}}},
        {"Lina",   {{"Thursday", {"Morning"}}, {"Saturday", {"Evening", "Morning"}}, {"Sunday", {"Afternoon"}}}},
    };
}

string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
    return s;
}

string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// Accept "m", "morning", "a", etc. Return full shift name or "" if invalid.
string parseShift(const string& text) {
    string t = toLower(trim(text));
    if (t.empty()) return "";
    for (const auto& s : SHIFTS)
        if (toLower(s).rfind(t, 0) == 0) return s;
    return "";
}

vector<Employee> collectInput() {
    vector<Employee> employees;
    int slots = MIN_PER_SHIFT * (int)SHIFTS.size() * (int)DAYS.size();
    int needed = (slots + MAX_DAYS - 1) / MAX_DAYS;  // ceiling division
    cout << "Note: " << MIN_PER_SHIFT << " employees/shift x " << SHIFTS.size() << " shifts x "
         << DAYS.size() << " days = " << slots << " shift slots per week.\n"
         << "      Each employee works at most " << MAX_DAYS << " days, so at least " << needed
         << " employees are needed to fill every shift.\n";
    int n;
    cout << "Number of employees (minimum " << needed << " recommended): ";
    cin >> n;
    cin.ignore();
    for (int i = 0; i < n; i++) {
        Employee e;
        cout << "\nEmployee name: ";
        getline(cin, e.name);
        e.name = trim(e.name);
        cout << "For each day, enter ranked shifts separated by commas "
                "(e.g. 'morning,evening' or 'm,e'), * for any shift, or blank if unavailable.\n";
        for (const auto& day : DAYS) {
            cout << "  " << day << ": ";
            string raw, part;
            getline(cin, raw);
            stringstream ss(raw);
            vector<string> ranked;
            if (trim(raw) == "*") ranked = SHIFTS;  // available for any shift that day
            while (getline(ss, part, ',')) {
                string s = parseShift(part);
                if (!s.empty() && find(ranked.begin(), ranked.end(), s) == ranked.end())
                    ranked.push_back(s);
            }
            if (!ranked.empty()) e.prefs[day] = ranked;
        }
        employees.push_back(e);
    }
    return employees;
}

class Scheduler {
public:
    int maxPerShift;
    Schedule schedule;
    map<string, int> daysWorked;
    vector<string> conflicts;

    explicit Scheduler(const vector<Employee>& emps, unsigned seed = 42, int maxPerShift = MAX_PER_SHIFT)
        : maxPerShift(maxPerShift), employees(emps), rng(seed) {
        for (const auto& day : DAYS)
            for (const auto& s : SHIFTS) schedule[day][s] = {};
        for (const auto& e : employees) daysWorked[e.name] = 0;
    }

    void build() {
        vector<string> carryOver;  // employees bumped from the previous day

        for (size_t i = 0; i < DAYS.size(); i++) {
            const string& day = DAYS[i];
            vector<string> nextCarry;

            // Bumped employees from yesterday go first, then everyone who wants today
            vector<pair<string, bool>> queue;
            for (const auto& n : carryOver) queue.push_back({n, true});
            for (const auto& e : employees)
                if (e.prefs.count(day) &&
                    find(carryOver.begin(), carryOver.end(), e.name) == carryOver.end())
                    queue.push_back({e.name, false});

            for (const auto& [name, carried] : queue) {
                if (!canWork(name, day)) continue;
                vector<string> ranked = prefsFor(name, day);
                bool placed = false;

                // 1. Try preferences in ranked order
                for (const auto& shift : ranked) {
                    if ((int)schedule[day][shift].size() < maxPerShift) {
                        assign(name, day, shift);
                        placed = true;
                        if (shift != ranked[0])
                            conflicts.push_back(day + ": " + name + "'s 1st choice (" + ranked[0] +
                                                ") was full -> placed in " + shift);
                        else if (carried)
                            conflicts.push_back(day + ": " + name +
                                                " carried over from previous day -> placed in " + shift);
                        break;
                    }
                }

                // 2. Conflict: any other shift with room on the same day
                if (!placed) {
                    for (const auto& shift : SHIFTS) {
                        if ((int)schedule[day][shift].size() < maxPerShift) {
                            assign(name, day, shift);
                            placed = true;
                            string reason = ranked.empty() ? "carried over" : "preferred shift(s) full";
                            conflicts.push_back(day + ": " + name + " " + reason + " -> placed in " + shift);
                            break;
                        }
                    }
                }

                // 3. Whole day full: carry to next day
                if (!placed) {
                    if (i + 1 < DAYS.size()) {
                        nextCarry.push_back(name);
                        conflicts.push_back(day + ": all shifts full for " + name + " -> moved to " + DAYS[i + 1]);
                    } else {
                        conflicts.push_back(day + ": all shifts full for " + name + ", no next day available");
                    }
                }
            }

            // Enforce minimum staffing with random eligible employees
            for (const auto& shift : SHIFTS) {
                while ((int)schedule[day][shift].size() < MIN_PER_SHIFT) {
                    // Favor employees with the most spare capacity: days already worked
                    // plus days they still want later this week. This avoids using up
                    // someone's 5 days before the days they actually asked for.
                    auto load = [&](const string& n) {
                        int count = daysWorked[n];
                        for (size_t j = i + 1; j < DAYS.size(); j++)
                            if (!prefsFor(n, DAYS[j]).empty()) count++;
                        return count;
                    };
                    // Only pull in someone if it won't use up a day they asked for later
                    vector<string> eligible;
                    for (const auto& e : employees)
                        if (canWork(e.name, day) && load(e.name) < MAX_DAYS) eligible.push_back(e.name);
                    if (eligible.empty()) {
                        conflicts.push_back(day + " " + shift + ": WARNING - not enough available employees");
                        break;
                    }
                    shuffle(eligible.begin(), eligible.end(), rng);
                    stable_sort(eligible.begin(), eligible.end(), [&](const string& a, const string& b) {
                        return load(a) < load(b);
                    });
                    assign(eligible[0], day, shift);
                    conflicts.push_back(day + " " + shift + ": understaffed -> randomly assigned " + eligible[0]);
                }
            }

            carryOver = nextCarry;
        }
    }

    void print() const {
        const int W = 36;
        string line = "+" + string(11, '-');
        for (size_t i = 0; i < SHIFTS.size(); i++) line += "+" + string(W, '-');
        line += "+";

        cout << "\n" << string(line.size(), '=') << "\n"
             << center("FINAL WEEKLY SCHEDULE (C++)", line.size()) << "\n"
             << string(line.size(), '=') << "\n"
             << line << "\n|" << left << setw(11) << "Day";
        for (const auto& s : SHIFTS) cout << "|" << center(s, W);
        cout << "|\n" << line << "\n";

        for (const auto& day : DAYS) {
            cout << "|" << left << setw(11) << day;
            for (const auto& s : SHIFTS) cout << "|" << center(join(schedule.at(day).at(s)), W);
            cout << "|\n";
        }
        cout << line << "\n\nDays worked per employee:\n";
        for (const auto& e : employees)
            cout << "  " << left << setw(8) << e.name << " " << daysWorked.at(e.name) << "\n";

        cout << "\nConflicts / adjustments:\n";
        for (const auto& c : conflicts) cout << "  - " << c << "\n";
    }

private:
    vector<Employee> employees;
    mt19937 rng;

    bool canWork(const string& name, const string& day) {
        for (const auto& s : SHIFTS) {
            const auto& v = schedule[day][s];
            if (find(v.begin(), v.end(), name) != v.end()) return false;
        }
        return daysWorked[name] < MAX_DAYS;
    }

    void assign(const string& name, const string& day, const string& shift) {
        schedule[day][shift].push_back(name);
        daysWorked[name]++;
    }

    vector<string> prefsFor(const string& name, const string& day) const {
        for (const auto& e : employees)
            if (e.name == name && e.prefs.count(day)) return e.prefs.at(day);
        return {};
    }

    static string join(const vector<string>& v) {
        string out;
        for (size_t i = 0; i < v.size(); i++) out += (i ? ", " : "") + v[i];
        return out;
    }

    static string center(const string& s, size_t w) {
        if (s.size() >= w) return s;
        size_t left = (w - s.size()) / 2;
        return string(left, ' ') + s + string(w - s.size() - left, ' ');
    }
};

#ifndef SCHEDULE_NO_MAIN  // lets test_schedule.cpp include this file
int main() {
    cout << "Use sample employee data? (y/n): ";
    string choice;
    getline(cin, choice);
    vector<Employee> employees;
    int maxPerShift = MAX_PER_SHIFT;
    if (toLower(trim(choice)) == "n") {
        employees = collectInput();
        cout << "\nMax employees per shift (press Enter for " << MAX_PER_SHIFT << "): ";
        string raw;
        getline(cin, raw);
        raw = trim(raw);
        if (!raw.empty() && all_of(raw.begin(), raw.end(), ::isdigit) && stoi(raw) >= MIN_PER_SHIFT)
            maxPerShift = stoi(raw);
    } else {
        employees = sampleData();
    }
    cout << "\nShift capacity: " << MIN_PER_SHIFT << "-" << maxPerShift << " employees per shift\n";

    Scheduler scheduler(employees, 42, maxPerShift);
    scheduler.build();
    scheduler.print();
    return 0;
}
#endif
