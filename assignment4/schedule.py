"""
Employee Shift Scheduler (Python)
MSCS 632 - Advanced Programming Languages

Rules:
  - 7 days, 3 shifts per day (Morning, Afternoon, Evening)
  - No employee works more than one shift per day
  - No employee works more than 5 days per week
  - At least 2 employees per shift; if short, randomly assign
    employees who have not yet worked 5 days
  - Shift capacity defaults to 5 (user-set in manual mode)
  - If a preferred shift is full (capacity reached), the employee is moved to
    another shift that day, or carried to the next day if the whole day is full
  - Preferences are ranked (bonus): earlier in the list = higher priority
"""

import random

DAYS = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"]
SHIFTS = ["Morning", "Afternoon", "Evening"]
MIN_PER_SHIFT = 2
MAX_PER_SHIFT = 5  # default shift capacity; the prompt requires shifts to be able to fill up but sets no limit
MAX_DAYS = 5

# Sample data: name -> {day: [ranked shift preferences]}
SAMPLE = {
    "Alice":  {"Monday": ["Morning", "Afternoon"], "Tuesday": ["Morning"], "Wednesday": ["Morning", "Evening"], "Friday": ["Morning"]},
    "Bob":    {"Monday": ["Morning", "Evening"], "Tuesday": ["Afternoon", "Evening"], "Thursday": ["Afternoon"], "Saturday": ["Afternoon", "Morning"]},
    "Carmen": {"Monday": ["Evening"], "Wednesday": ["Evening"], "Thursday": ["Evening", "Afternoon"], "Sunday": ["Evening"]},
    "David":  {"Monday": ["Morning"], "Tuesday": ["Morning", "Afternoon"], "Thursday": ["Morning"], "Saturday": ["Morning"]},
    "Esha":   {"Monday": ["Morning", "Afternoon"], "Tuesday": ["Evening"], "Friday": ["Afternoon", "Morning"], "Sunday": ["Morning", "Evening"]},
    "Farid":  {"Monday": ["Morning", "Afternoon"], "Wednesday": ["Afternoon"], "Friday": ["Evening"], "Sunday": ["Afternoon"]},
    "Grace":  {"Tuesday": ["Afternoon", "Morning"], "Wednesday": ["Afternoon", "Morning"], "Friday": ["Morning"], "Saturday": ["Evening"]},
    "Hiro":   {"Tuesday": ["Evening", "Afternoon"], "Thursday": ["Evening"], "Saturday": ["Morning"], "Sunday": ["Evening"]},
    "Isabel": {"Wednesday": ["Afternoon"], "Thursday": ["Afternoon"], "Saturday": ["Evening", "Afternoon"], "Sunday": ["Morning"]},
    "Jamal":  {"Wednesday": ["Morning"], "Thursday": ["Morning", "Evening"], "Friday": ["Evening", "Afternoon"], "Sunday": ["Afternoon", "Morning"]},
    "Kenji":  {"Monday": ["Morning", "Evening"], "Tuesday": ["Morning"], "Friday": ["Afternoon"], "Saturday": ["Afternoon"]},
    "Lina":   {"Thursday": ["Morning"], "Saturday": ["Evening", "Morning"], "Sunday": ["Afternoon"]},
}


def parse_shift(text):
    """Accept 'm', 'morning', 'a', etc. Return the full shift name or None."""
    text = text.strip().lower()
    for s in SHIFTS:
        if text and s.lower().startswith(text):
            return s
    return None


def collect_input():
    """Interactively collect employee names and ranked preferences."""
    employees = {}
    slots = MIN_PER_SHIFT * len(SHIFTS) * len(DAYS)
    needed = -(-slots // MAX_DAYS)  # ceiling division
    print(f"Note: {MIN_PER_SHIFT} employees/shift x {len(SHIFTS)} shifts x {len(DAYS)} days = {slots} shift slots per week.")
    print(f"      Each employee works at most {MAX_DAYS} days, so at least {needed} employees are needed to fill every shift.")
    n = int(input(f"Number of employees (minimum {needed} recommended): "))
    for _ in range(n):
        name = input("\nEmployee name: ").strip()
        prefs = {}
        print("For each day, enter ranked shifts separated by commas "
              "(e.g. 'morning,evening' or 'm,e'), * for any shift, or blank if unavailable.")
        for day in DAYS:
            raw = input(f"  {day}: ")
            ranked = []
            if raw.strip() == "*":
                ranked = list(SHIFTS)  # available for any shift that day
            for part in raw.split(","):
                s = parse_shift(part)
                if s and s not in ranked:
                    ranked.append(s)
            if ranked:
                prefs[day] = ranked
        employees[name] = prefs
    return employees


def build_schedule(employees, seed=42, max_per_shift=MAX_PER_SHIFT):
    rng = random.Random(seed)
    schedule = {day: {s: [] for s in SHIFTS} for day in DAYS}
    days_worked = {name: 0 for name in employees}
    conflicts = []
    carry_over = []  # employees bumped from the previous day

    def can_work(name, day):
        assigned_today = any(name in schedule[day][s] for s in SHIFTS)
        return not assigned_today and days_worked[name] < MAX_DAYS

    def assign(name, day, shift):
        schedule[day][shift].append(name)
        days_worked[name] += 1

    for i, day in enumerate(DAYS):
        next_carry = []

        # Bumped employees from yesterday go first, then everyone who wants today
        queue = [(n, True) for n in carry_over]
        queue += [(n, False) for n, p in employees.items() if day in p and n not in carry_over]

        for name, carried in queue:
            if not can_work(name, day):
                continue
            ranked = employees[name].get(day, [])
            placed = False

            # 1. Try preferences in ranked order
            for shift in ranked:
                if len(schedule[day][shift]) < max_per_shift:
                    assign(name, day, shift)
                    placed = True
                    if shift != ranked[0]:
                        conflicts.append(f"{day}: {name}'s 1st choice ({ranked[0]}) was full -> placed in {shift}")
                    elif carried:
                        conflicts.append(f"{day}: {name} carried over from previous day -> placed in {shift}")
                    break

            # 2. Conflict: any other shift with room on the same day
            if not placed:
                for shift in SHIFTS:
                    if len(schedule[day][shift]) < max_per_shift:
                        assign(name, day, shift)
                        placed = True
                        reason = f"preferred shift(s) {ranked} full" if ranked else "carried over"
                        conflicts.append(f"{day}: {name} {reason} -> placed in {shift}")
                        break

            # 3. Whole day full: carry to next day
            if not placed:
                if i + 1 < len(DAYS):
                    next_carry.append(name)
                    conflicts.append(f"{day}: all shifts full for {name} -> moved to {DAYS[i + 1]}")
                else:
                    conflicts.append(f"{day}: all shifts full for {name}, no next day available")

        # Enforce minimum staffing with random eligible employees
        for shift in SHIFTS:
            while len(schedule[day][shift]) < MIN_PER_SHIFT:
                # Only pull in someone if it won't use up a day they asked for later
                later = DAYS[i + 1:]
                eligible = [n for n in employees if can_work(n, day) and
                            days_worked[n] + sum(1 for d in later if d in employees[n]) < MAX_DAYS]
                if not eligible:
                    conflicts.append(f"{day} {shift}: WARNING - not enough available employees")
                    break
                rng.shuffle(eligible)
                # Favor employees with the most spare capacity: days already worked
                # plus days they still want later this week. This avoids using up
                # someone's 5 days before the days they actually asked for.
                eligible.sort(key=lambda n: days_worked[n] +
                              sum(1 for d in later if d in employees[n]))
                pick = eligible[0]
                assign(pick, day, shift)
                conflicts.append(f"{day} {shift}: understaffed -> randomly assigned {pick}")

        carry_over = next_carry

    return schedule, days_worked, conflicts


def print_schedule(schedule, days_worked, conflicts):
    width = 36
    line = "+" + "+".join(["-" * 11] + ["-" * width] * len(SHIFTS)) + "+"
    print("\n" + "=" * len(line))
    print("FINAL WEEKLY SCHEDULE (Python)".center(len(line)))
    print("=" * len(line))
    print(line)
    print("|" + "Day".ljust(11) + "|" + "|".join(s.center(width) for s in SHIFTS) + "|")
    print(line)
    for day in DAYS:
        cells = [", ".join(schedule[day][s]).center(width) for s in SHIFTS]
        print("|" + day.ljust(11) + "|" + "|".join(cells) + "|")
    print(line)

    print("\nDays worked per employee:")
    for name, d in days_worked.items():
        print(f"  {name:<8} {d}")

    print("\nConflicts / adjustments:")
    for c in conflicts:
        print("  - " + c)


def main():
    choice = input("Use sample employee data? (y/n): ").strip().lower()
    if choice == "n":
        employees = collect_input()
        raw = input(f"\nMax employees per shift (press Enter for {MAX_PER_SHIFT}): ").strip()
        max_per_shift = int(raw) if raw.isdigit() and int(raw) >= MIN_PER_SHIFT else MAX_PER_SHIFT
    else:
        employees, max_per_shift = SAMPLE, MAX_PER_SHIFT
    print(f"\nShift capacity: {MIN_PER_SHIFT}-{max_per_shift} employees per shift")
    schedule, days_worked, conflicts = build_schedule(employees, max_per_shift=max_per_shift)
    print_schedule(schedule, days_worked, conflicts)


if __name__ == "__main__":
    main()
