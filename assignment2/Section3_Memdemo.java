//Section 3 Memory Demo for Java

import java.util.ArrayList;
import java.util.List;

public class Section3_Memdemo {
    // Static list simulates a "leak": objects stay reachable forever,
    // so the GC can never reclaim them even though they're never used again.
    static List<int[]> retained = new ArrayList<>();

    public static void main(String[] args) throws InterruptedException {
        Runtime rt = Runtime.getRuntime();

        System.out.println("Before: used = " + used(rt) + " bytes");

        for (int round = 0; round < 5; round++) {
            for (int i = 0; i < 20000; i++) {
                retained.add(new int[100]); // 400 bytes each, never removed
            }
            System.out.println("After round " + round + ": used = " + used(rt) + " bytes"
                + " (retained size=" + retained.size() + ")");
            Thread.sleep(200); // give FlightRecorder time to sample
        }

        System.out.println("Requesting GC (hint only)...");
        System.gc();
        Thread.sleep(200);
        System.out.println("After System.gc(): used = " + used(rt) + " bytes");
        System.out.println("Note: memory did NOT drop back down -- 'retained' is still reachable,");
        System.out.println("so the GC correctly refuses to collect it. This is the Java equivalent");
        System.out.println("of a leak: not a bug in the GC, but objects the programmer forgot to release.");
    }

    static long used(Runtime rt) {
        return rt.totalMemory() - rt.freeMemory();
    }
}