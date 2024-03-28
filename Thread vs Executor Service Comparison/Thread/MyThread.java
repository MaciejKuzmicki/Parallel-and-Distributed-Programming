public class MyThread implements Runnable {
    private final Counter counter;
    private final boolean sync;

    public MyThread(Counter counter, boolean sync) {
        this.counter = counter;
        this.sync = sync;
    }

    @Override
    public void run() {
        if(sync) {
            for(int i = 0; i < 1000; i++) {
                counter.incrementSync();
            }
        }
        else {
            for(int i = 0; i < 1000; i++) {
                counter.increment();
            }
        }
    }
}