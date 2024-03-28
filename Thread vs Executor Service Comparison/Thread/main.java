public class main {
    public static void main(String[] args) {
        int n = Integer.parseInt(args[0]);
        boolean s = Boolean.parseBoolean(args[1]);
        Thread[] threads = new Thread[n];
        Counter counter = new Counter();

        for(int i = 0; i < n; i++) {
            threads[i] = new Thread(new MyThread(counter, s));
            threads[i].start();
        }
        try {
            for (Thread thread : threads) {
                thread.join();
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        System.out.println("Final counter value: " + counter.getCount());
    }
}
