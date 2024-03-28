import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class main {
    public static void main(String[] args) throws InterruptedException {
        int n = Integer.parseInt(args[0]);
        boolean s = Boolean.parseBoolean(args[1]);
        Counter counter = new Counter();
        ExecutorService executor = Executors.newFixedThreadPool(n);

        for (int i = 0; i < n; i++) {
            executor.execute(new MyThread(counter, s));
        }
        executor.shutdown();
        if(executor.awaitTermination(5, TimeUnit.SECONDS)) {
            executor.shutdownNow();
        }
        System.out.println("Final counter value: " + counter.getCount());
    }
}
