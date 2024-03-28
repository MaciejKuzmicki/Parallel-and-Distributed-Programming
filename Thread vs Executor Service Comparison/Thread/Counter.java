public class Counter {
    private int count = 0;
    public synchronized void incrementSync() {
        count++;
    }
    public void increment() {
        count++;
    }
    public int getCount() {
        return count;
    }
}
