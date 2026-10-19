# Employee Shift Scheduler (Python & C++)

MSCS 632 – Advanced Programming Languages

This repository implements the same weekly shift scheduler in two languages: Python (`schedule.py`) and C++ (`schedule.cpp`). Each program collects employee names and their shift preferences for each day of the week, or loads built-in sample data. It then builds a 7-day schedule across three shifts (Morning, Afternoon, Evening) and prints the final schedule, the number of days each employee worked, and a log of every conflict or adjustment it made.

## Scheduling rules

The scheduler enforces the following rules. No employee works more than one shift per day, and no employee works more than 5 days per week. Each shift must have at least 2 employees. The prompt doesn't specify a maximum, but conflict resolution requires that a preferred shift can become "full," so the program uses a shift capacity. It defaults to 5 employees per shift (the `MAX_PER_SHIFT` constant in either file), and in manual-entry mode the program asks the user to set it, using 5 if they press Enter. If a shift has fewer than 2 employees after preferences are processed, the program randomly assigns additional employees who have not yet reached 5 days. It skips anyone whose remaining days are already reserved for days they asked to work later in the week, so the random fill never uses up an employee's week before the days they actually requested. If the shift still can't be staffed, the program prints a warning.

## Ranked shift preferences (bonus)

Employees can rank their shift preferences for each day instead of choosing only one. For example, `morning,evening` means Morning is their first choice and Evening their second. The scheduler always tries an employee's choices in ranked order, so a lower-ranked choice is used only when every higher-ranked one is full. This lets the program honor preferences as closely as possible while still meeting the company's staffing requirements.

## Conflict resolution

A conflict occurs when an employee's preferred shift is already full for a given day. The program resolves it in three steps. First, it tries the employee's remaining ranked choices for that day. If all of those are full, it places the employee in any other shift that still has room on the same day. If every shift on that day is full, it carries the employee over to the next day, where carried-over employees are scheduled first and receive their preferred shift for that day if they listed one. A full Sunday has no next day, so that case is recorded in the log. Every resolution appears in the "Conflicts / adjustments" section of the output.

## Running the programs

Python 3:
```
python3 schedule.py
```

C++ (C++17):
```
g++ -std=c++17 -o schedule schedule.cpp
./schedule
```

At the prompt, enter `y` to use the sample data (12 employees) or `n` to enter employees manually. For manual entry, list each day's shifts in ranked order separated by commas (full names or first letters: `m` for Morning, `a` for Afternoon, `e` for Evening, such as `m,a,e`), enter `*` if the employee can work any shift that day (ranked Morning, Afternoon, Evening), and leave a day blank if the employee is unavailable. Fully staffing the week requires at least 9 employees: 7 days × 3 shifts × 2 employees is 42 shift slots, and each employee can cover at most 5 of them. With fewer employees, the program still schedules as much as it can and flags the shifts it can't fill. Manual mode displays this calculation before asking for the number of employees, then asks for the maximum employees per shift after all preferences are entered.

Both versions use a fixed random seed, so runs are repeatable. The Python and C++ random number generators differ, which means the randomly filled names can vary between the two languages. All of the rule-based logic is identical in both.

## Unit tests

Both implementations include the same 12 unit tests, focused mainly on conflict detection and resolution: `test_schedule.py` for Python and `test_schedule.cpp` for C++. The C++ tests include `schedule.cpp` directly with its `main` function disabled, so no testing framework is required. Run them from the repository folder with:

```
python3 -m unittest test_schedule.py -v
```

```
g++ -std=c++17 -o test_schedule test_schedule.cpp
./test_schedule
```

Every test that builds a schedule also checks the core invariants: no shift exceeds capacity, no employee works twice on the same day, and no employee works more than 5 days.

| # | Test | What it verifies |
|---|------|------------------|
| 1 | `test_no_conflict_gets_first_choice` | With no competition, each employee gets their first choice and no conflict is logged. |
| 2 | `test_first_choice_full_falls_to_second_choice` | When the first choice is full, the employee receives their second choice and the conflict is logged. |
| 3 | `test_ranked_order_is_respected` | With three ranked choices and the first one full, the employee gets their second choice rather than their third. |
| 4 | `test_all_preferences_full_moves_to_other_shift_same_day` | When every preferred shift is full, the employee is placed in another open shift on the same day. |
| 5 | `test_whole_day_full_moves_to_next_day` | When all three shifts on a day are full, the employee is carried over to the next day and scheduled there. |
| 6 | `test_carried_employee_gets_their_next_day_preference` | A carried-over employee receives the shift they requested for the next day. |
| 7 | `test_carried_employee_has_priority_next_day` | A carried-over employee is scheduled before others on the next day, which can move one of those employees to another shift. |
| 8 | `test_sunday_full_has_no_next_day` | When Sunday is full, the program logs that no next day is available instead of failing. |
| 9 | `test_cascading_conflicts_across_days` | When consecutive days are full, conflicts carry forward correctly from one day to the next. |
| 10 | `test_employee_at_max_days_is_not_scheduled_again` | An employee who requests all 7 days is scheduled for only 5. |
| 11 | `test_heavy_contention_random_inputs` | A stress test over 50 random rosters of 5–20 employees confirms that all invariants hold. |
| 12 | `test_sample_data` | The sample data produces a valid schedule with at least 2 employees on every shift, including the expected Monday conflict (Kenji's first choice, Morning, is full, so he is placed in Evening). |
