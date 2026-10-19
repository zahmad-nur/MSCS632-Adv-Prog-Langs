"""
Tests for the shift-conflict logic in schedule.py.
Run: python3 -m unittest test_schedule.py -v
"""

import unittest
from schedule import build_schedule, DAYS, SHIFTS, SAMPLE, MAX_PER_SHIFT, MAX_DAYS


def shift_of(schedule, name, day):
    """Return the shift `name` works on `day`, or None."""
    for s in SHIFTS:
        if name in schedule[day][s]:
            return s
    return None


def check_invariants(test, schedule, days_worked, employees):
    """Rules that must hold for every schedule."""
    for day in DAYS:
        for s in SHIFTS:
            test.assertLessEqual(len(schedule[day][s]), MAX_PER_SHIFT, f"{day} {s} over capacity")
        names_today = [n for s in SHIFTS for n in schedule[day][s]]
        test.assertEqual(len(names_today), len(set(names_today)), f"someone works twice on {day}")
    for name in employees:
        actual = sum(1 for d in DAYS if shift_of(schedule, name, d))
        test.assertEqual(actual, days_worked[name])
        test.assertLessEqual(actual, MAX_DAYS, f"{name} works more than {MAX_DAYS} days")


class TestConflictResolution(unittest.TestCase):

    def test_no_conflict_gets_first_choice(self):
        emps = {"A": {"Monday": ["Morning"]}, "B": {"Monday": ["Evening"]}}
        sched, worked, conflicts = build_schedule(emps)
        self.assertEqual(shift_of(sched, "A", "Monday"), "Morning")
        self.assertEqual(shift_of(sched, "B", "Monday"), "Evening")
        self.assertFalse(any("full" in c for c in conflicts))

    def test_first_choice_full_falls_to_second_choice(self):
        # Morning is filled to capacity; one more ranks Morning then Evening
        emps = {f"M{i}": {"Monday": ["Morning"]} for i in range(MAX_PER_SHIFT)}
        emps["X"] = {"Monday": ["Morning", "Evening"]}
        sched, worked, conflicts = build_schedule(emps)
        self.assertEqual(len(sched["Monday"]["Morning"]), MAX_PER_SHIFT)
        self.assertEqual(shift_of(sched, "X", "Monday"), "Evening")
        self.assertTrue(any("X's 1st choice (Morning) was full -> placed in Evening" in c for c in conflicts))
        check_invariants(self, sched, worked, emps)

    def test_ranked_order_is_respected(self):
        # Morning full; X ranks Morning > Evening > Afternoon -> should get Evening, not Afternoon
        emps = {f"M{i}": {"Monday": ["Morning"]} for i in range(MAX_PER_SHIFT)}
        emps["X"] = {"Monday": ["Morning", "Evening", "Afternoon"]}
        sched, _, _ = build_schedule(emps)
        self.assertEqual(shift_of(sched, "X", "Monday"), "Evening")

    def test_all_preferences_full_moves_to_other_shift_same_day(self):
        # Morning and Evening both full; X only wants those -> placed in Afternoon same day
        emps = {f"M{i}": {"Monday": ["Morning"]} for i in range(MAX_PER_SHIFT)}
        emps.update({f"E{i}": {"Monday": ["Evening"]} for i in range(MAX_PER_SHIFT)})
        emps["X"] = {"Monday": ["Morning", "Evening"]}
        sched, worked, conflicts = build_schedule(emps)
        self.assertEqual(shift_of(sched, "X", "Monday"), "Afternoon")
        self.assertTrue(any("X preferred shift(s)" in c and "placed in Afternoon" in c for c in conflicts))
        check_invariants(self, sched, worked, emps)

    def test_whole_day_full_moves_to_next_day(self):
        # Monday is filled to capacity on every shift; one more wants Monday -> moved to Tuesday
        emps = {}
        for s in SHIFTS:
            for i in range(MAX_PER_SHIFT):
                emps[f"{s[0]}{i}"] = {"Monday": [s]}
        emps["X"] = {"Monday": ["Morning"]}
        sched, worked, conflicts = build_schedule(emps)
        self.assertIsNone(shift_of(sched, "X", "Monday"))
        self.assertIsNotNone(shift_of(sched, "X", "Tuesday"))
        self.assertTrue(any("all shifts full for X -> moved to Tuesday" in c for c in conflicts))
        self.assertTrue(any("Tuesday: X carried over" in c for c in conflicts))
        check_invariants(self, sched, worked, emps)

    def test_carried_employee_gets_their_next_day_preference(self):
        # X is bumped from Monday and also wants Tuesday Evening -> should land in Tuesday Evening
        emps = {}
        for s in SHIFTS:
            for i in range(MAX_PER_SHIFT):
                emps[f"{s[0]}{i}"] = {"Monday": [s]}
        emps["X"] = {"Monday": ["Morning"], "Tuesday": ["Evening"]}
        sched, worked, _ = build_schedule(emps)
        self.assertEqual(shift_of(sched, "X", "Tuesday"), "Evening")
        check_invariants(self, sched, worked, emps)

    def test_carried_employee_has_priority_next_day(self):
        # Monday full -> X carried. On Tuesday, enough others want Morning to fill it, as does X.
        # Morning is fully requested by others too. X goes first, so X gets Tuesday Morning and one of the others is bumped.
        emps = {}
        for s in SHIFTS:
            for i in range(MAX_PER_SHIFT):
                emps[f"{s[0]}{i}"] = {"Monday": [s]}
        emps["X"] = {"Monday": ["Morning"], "Tuesday": ["Morning"]}
        emps.update({f"T{i}": {"Tuesday": ["Morning"]} for i in range(MAX_PER_SHIFT)})
        sched, worked, conflicts = build_schedule(emps)
        self.assertEqual(shift_of(sched, "X", "Tuesday"), "Morning")
        self.assertEqual(len(sched["Tuesday"]["Morning"]), MAX_PER_SHIFT)
        bumped = [n for n in emps if n.startswith("T") and shift_of(sched, n, "Tuesday") != "Morning"]
        self.assertEqual(len(bumped), 1)
        check_invariants(self, sched, worked, emps)

    def test_sunday_full_has_no_next_day(self):
        emps = {}
        for s in SHIFTS:
            for i in range(MAX_PER_SHIFT):
                emps[f"{s[0]}{i}"] = {"Sunday": [s]}
        emps["X"] = {"Sunday": ["Morning"]}
        sched, worked, conflicts = build_schedule(emps)
        self.assertIsNone(shift_of(sched, "X", "Sunday"))
        self.assertTrue(any("all shifts full for X, no next day available" in c for c in conflicts))
        check_invariants(self, sched, worked, emps)

    def test_cascading_conflicts_across_days(self):
        # Monday and Tuesday are both fully booked by others; X and Y want Monday.
        # Bumped to Tuesday, which is also full -> bumped again to Wednesday.
        emps = {}
        for day in ("Monday", "Tuesday"):
            for s in SHIFTS:
                for i in range(MAX_PER_SHIFT):
                    emps[f"{day[:2]}{s[0]}{i}"] = {day: [s]}
        emps["X"] = {"Monday": ["Afternoon"]}
        sched, worked, conflicts = build_schedule(emps)
        # X is carried onto Tuesday and goes FIRST, so X gets a Tuesday slot
        # and one of the Tuesday people is pushed to Wednesday instead.
        self.assertIsNotNone(shift_of(sched, "X", "Tuesday"))
        self.assertTrue(any("moved to Wednesday" in c for c in conflicts))
        check_invariants(self, sched, worked, emps)

    def test_employee_at_max_days_is_not_scheduled_again(self):
        # X wants all 7 days -> only 5 are scheduled
        emps = {"X": {d: ["Morning"] for d in DAYS}}
        sched, worked, _ = build_schedule(emps)
        self.assertEqual(worked["X"], MAX_DAYS)
        check_invariants(self, sched, worked, emps)

    def test_heavy_contention_random_inputs(self):
        # Stress test: many employees all fighting for the same shift, several seeds
        import random
        for seed in range(50):
            r = random.Random(seed)
            emps = {}
            for i in range(r.randint(5, 20)):
                prefs = {}
                for d in r.sample(DAYS, r.randint(1, 7)):
                    prefs[d] = r.sample(SHIFTS, r.randint(1, 3))
                emps[f"E{i}"] = prefs
            sched, worked, _ = build_schedule(emps, seed=seed)
            check_invariants(self, sched, worked, emps)

    def test_sample_data(self):
        sched, worked, conflicts = build_schedule(SAMPLE)
        check_invariants(self, sched, worked, SAMPLE)
        for day in DAYS:
            for s in SHIFTS:
                self.assertGreaterEqual(len(sched[day][s]), 2, f"{day} {s} understaffed")
        self.assertEqual(shift_of(sched, "Kenji", "Monday"), "Evening")  # Morning was full


if __name__ == "__main__":
    unittest.main()
