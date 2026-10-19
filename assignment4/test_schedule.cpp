/*
 * Tests for the shift-conflict logic in schedule.cpp.
 * Mirrors the 12 tests in test_schedule.py.
 *
 * Build & run:
 *   g++ -std=c++17 -o test_schedule test_schedule.cpp && ./test_schedule
 */

#define SCHEDULE_NO_MAIN
#include "schedule.cpp"

#include <functional>

static int failures = 0;
static string currentTest;

#define CHECK(cond, msg)                                                  \
    do {                                                                  \
        if (!(cond)) {                                                    \
            cout << "    FAIL: " << msg << " (line " << __LINE__ << ")\n"; \
            failures++;                                                   \
        }                                                                 \
    } while (0)

// ---------- helpers ----------

string shiftOf(const Scheduler& s, const string& name, const string& day) {
    for (const auto& sh : SHIFTS) {
        const auto& v = s.schedule.at(day).at(sh);
        if (find(v.begin(), v.end(), name) != v.end()) return sh;
    }
    return "";
}

bool logContains(const Scheduler& s, const string& text) {
    for (const auto& c : s.conflicts)
        if (c.find(text) != string::npos) return true;
    return false;
}

// Rules that must hold for every schedule
void checkInvariants(const Scheduler& s, const vector<Employee>& emps) {
    for (const auto& day : DAYS) {
        vector<string> today;
        for (const auto& sh : SHIFTS) {
            const auto& v = s.schedule.at(day).at(sh);
            CHECK((int)v.size() <= MAX_PER_SHIFT, day + " " + sh + " over capacity");
            today.insert(today.end(), v.begin(), v.end());
        }
        vector<string> sorted = today;
        sort(sorted.begin(), sorted.end());
        CHECK(adjacent_find(sorted.begin(), sorted.end()) == sorted.end(), "someone works twice on " + day);
    }
    for (const auto& e : emps) {
        int actual = 0;
        for (const auto& day : DAYS)
            if (!shiftOf(s, e.name, day).empty()) actual++;
        CHECK(actual == s.daysWorked.at(e.name), e.name + " days-worked count mismatch");
        CHECK(actual <= MAX_DAYS, e.name + " works more than 5 days");
    }
}

Scheduler run(const vector<Employee>& emps, unsigned seed = 42) {
    Scheduler s(emps, seed);
    s.build();
    return s;
}

// MAX_PER_SHIFT employees per shift, filling all of `day`
void fillDay(vector<Employee>& emps, const string& day, const string& prefix = "") {
    for (const auto& sh : SHIFTS)
        for (int i = 0; i < MAX_PER_SHIFT; i++)
            emps.push_back({prefix + sh.substr(0, 1) + to_string(i), {{day, {sh}}}});
}

// ---------- tests ----------

void test_no_conflict_gets_first_choice() {
    vector<Employee> emps = {{"A", {{"Monday", {"Morning"}}}}, {"B", {{"Monday", {"Evening"}}}}};
    auto s = run(emps);
    CHECK(shiftOf(s, "A", "Monday") == "Morning", "A should get Morning");
    CHECK(shiftOf(s, "B", "Monday") == "Evening", "B should get Evening");
    CHECK(!logContains(s, "full"), "no 'full' conflict should be logged");
}

void test_first_choice_full_falls_to_second_choice() {
    vector<Employee> emps;
    for (int i = 0; i < MAX_PER_SHIFT; i++) emps.push_back({"M" + to_string(i), {{"Monday", {"Morning"}}}});
    emps.push_back({"X", {{"Monday", {"Morning", "Evening"}}}});
    auto s = run(emps);
    CHECK(s.schedule.at("Monday").at("Morning").size() == (size_t)MAX_PER_SHIFT, "Morning should be full");
    CHECK(shiftOf(s, "X", "Monday") == "Evening", "X should get 2nd choice Evening");
    CHECK(logContains(s, "X's 1st choice (Morning) was full -> placed in Evening"), "conflict logged");
    checkInvariants(s, emps);
}

void test_ranked_order_is_respected() {
    vector<Employee> emps;
    for (int i = 0; i < MAX_PER_SHIFT; i++) emps.push_back({"M" + to_string(i), {{"Monday", {"Morning"}}}});
    emps.push_back({"X", {{"Monday", {"Morning", "Evening", "Afternoon"}}}});
    auto s = run(emps);
    CHECK(shiftOf(s, "X", "Monday") == "Evening", "X should get Evening (2nd), not Afternoon (3rd)");
}

void test_all_preferences_full_moves_to_other_shift_same_day() {
    vector<Employee> emps;
    for (int i = 0; i < MAX_PER_SHIFT; i++) emps.push_back({"M" + to_string(i), {{"Monday", {"Morning"}}}});
    for (int i = 0; i < MAX_PER_SHIFT; i++) emps.push_back({"E" + to_string(i), {{"Monday", {"Evening"}}}});
    emps.push_back({"X", {{"Monday", {"Morning", "Evening"}}}});
    auto s = run(emps);
    CHECK(shiftOf(s, "X", "Monday") == "Afternoon", "X should be moved to Afternoon same day");
    CHECK(logContains(s, "X preferred shift(s) full -> placed in Afternoon"), "conflict logged");
    checkInvariants(s, emps);
}

void test_whole_day_full_moves_to_next_day() {
    vector<Employee> emps;
    fillDay(emps, "Monday");
    emps.push_back({"X", {{"Monday", {"Morning"}}}});
    auto s = run(emps);
    CHECK(shiftOf(s, "X", "Monday").empty(), "X should not work Monday");
    CHECK(!shiftOf(s, "X", "Tuesday").empty(), "X should work Tuesday");
    CHECK(logContains(s, "all shifts full for X -> moved to Tuesday"), "carry-over logged");
    CHECK(logContains(s, "Tuesday: X carried over"), "placement on Tuesday logged");
    checkInvariants(s, emps);
}

void test_carried_employee_gets_their_next_day_preference() {
    vector<Employee> emps;
    fillDay(emps, "Monday");
    emps.push_back({"X", {{"Monday", {"Morning"}}, {"Tuesday", {"Evening"}}}});
    auto s = run(emps);
    CHECK(shiftOf(s, "X", "Tuesday") == "Evening", "X should get Tuesday Evening");
    checkInvariants(s, emps);
}

void test_carried_employee_has_priority_next_day() {
    vector<Employee> emps;
    fillDay(emps, "Monday");
    emps.push_back({"X", {{"Monday", {"Morning"}}, {"Tuesday", {"Morning"}}}});
    for (int i = 0; i < MAX_PER_SHIFT; i++) emps.push_back({"T" + to_string(i), {{"Tuesday", {"Morning"}}}});
    auto s = run(emps);
    CHECK(shiftOf(s, "X", "Tuesday") == "Morning", "X should get Tuesday Morning first");
    CHECK(s.schedule.at("Tuesday").at("Morning").size() == (size_t)MAX_PER_SHIFT, "Tuesday Morning should be full");
    int bumped = 0;
    for (int i = 0; i < MAX_PER_SHIFT; i++)
        if (shiftOf(s, "T" + to_string(i), "Tuesday") != "Morning") bumped++;
    CHECK(bumped == 1, "exactly one Tuesday employee should be bumped");
    checkInvariants(s, emps);
}

void test_sunday_full_has_no_next_day() {
    vector<Employee> emps;
    fillDay(emps, "Sunday");
    emps.push_back({"X", {{"Sunday", {"Morning"}}}});
    auto s = run(emps);
    CHECK(shiftOf(s, "X", "Sunday").empty(), "X should not work Sunday");
    CHECK(logContains(s, "all shifts full for X, no next day available"), "no-next-day logged");
    checkInvariants(s, emps);
}

void test_cascading_conflicts_across_days() {
    vector<Employee> emps;
    fillDay(emps, "Monday", "Mo");
    fillDay(emps, "Tuesday", "Tu");
    emps.push_back({"X", {{"Monday", {"Afternoon"}}}});
    auto s = run(emps);
    CHECK(!shiftOf(s, "X", "Tuesday").empty(), "X should get a Tuesday slot");
    CHECK(logContains(s, "moved to Wednesday"), "a Tuesday employee should cascade to Wednesday");
    checkInvariants(s, emps);
}

void test_employee_at_max_days_is_not_scheduled_again() {
    Employee x{"X", {}};
    for (const auto& d : DAYS) x.prefs[d] = {"Morning"};
    vector<Employee> emps = {x};
    auto s = run(emps);
    CHECK(s.daysWorked.at("X") == MAX_DAYS, "X should work exactly 5 days");
    checkInvariants(s, emps);
}

void test_heavy_contention_random_inputs() {
    for (unsigned seed = 0; seed < 50; seed++) {
        mt19937 r(seed);
        auto randInt = [&](int lo, int hi) { return uniform_int_distribution<int>(lo, hi)(r); };
        vector<Employee> emps;
        int n = randInt(5, 20);
        for (int i = 0; i < n; i++) {
            Employee e{"E" + to_string(i), {}};
            vector<string> days = DAYS;
            shuffle(days.begin(), days.end(), r);
            days.resize(randInt(1, 7));
            for (const auto& d : days) {
                vector<string> sh = SHIFTS;
                shuffle(sh.begin(), sh.end(), r);
                sh.resize(randInt(1, 3));
                e.prefs[d] = sh;
            }
            emps.push_back(e);
        }
        auto s = run(emps, seed);
        checkInvariants(s, emps);
    }
}

void test_sample_data() {
    auto emps = sampleData();
    auto s = run(emps);
    checkInvariants(s, emps);
    for (const auto& day : DAYS)
        for (const auto& sh : SHIFTS)
            CHECK(s.schedule.at(day).at(sh).size() >= 2, day + " " + sh + " understaffed");
    CHECK(shiftOf(s, "Kenji", "Monday") == "Evening", "Kenji should be moved off full Morning");
}

int main() {
    vector<pair<string, function<void()>>> tests = {
        {"test_no_conflict_gets_first_choice", test_no_conflict_gets_first_choice},
        {"test_first_choice_full_falls_to_second_choice", test_first_choice_full_falls_to_second_choice},
        {"test_ranked_order_is_respected", test_ranked_order_is_respected},
        {"test_all_preferences_full_moves_to_other_shift_same_day", test_all_preferences_full_moves_to_other_shift_same_day},
        {"test_whole_day_full_moves_to_next_day", test_whole_day_full_moves_to_next_day},
        {"test_carried_employee_gets_their_next_day_preference", test_carried_employee_gets_their_next_day_preference},
        {"test_carried_employee_has_priority_next_day", test_carried_employee_has_priority_next_day},
        {"test_sunday_full_has_no_next_day", test_sunday_full_has_no_next_day},
        {"test_cascading_conflicts_across_days", test_cascading_conflicts_across_days},
        {"test_employee_at_max_days_is_not_scheduled_again", test_employee_at_max_days_is_not_scheduled_again},
        {"test_heavy_contention_random_inputs", test_heavy_contention_random_inputs},
        {"test_sample_data", test_sample_data},
    };

    int passed = 0;
    for (const auto& [name, fn] : tests) {
        int before = failures;
        fn();
        bool ok = failures == before;
        if (ok) passed++;
        cout << name << " ... " << (ok ? "ok" : "FAILED") << "\n";
    }
    cout << "\nRan " << tests.size() << " tests: " << passed << " passed, "
         << (tests.size() - passed) << " failed\n";
    return passed == (int)tests.size() ? 0 : 1;
}
