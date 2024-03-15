public class MatrixThreadMultiplier implements Runnable {
    private final Matrix A, B, C;
    private final int start, end;

    public MatrixThreadMultiplier(Matrix A, Matrix B, Matrix C, int start, int end)  {
        this.A = A;
        this.B = B;
        this.C = C;
        this.start = start;
        this.end = end;
    }
    @Override
    public void run() {
        for (int i = start; i <= end; i++) {
            int row = i / B.cols();
            int col = i % B.cols();
            float sum = 0;
            for (int k = 0; k < A.cols(); k++) {
                sum += A.get(row, k) * B.get(k, col);
            }
            synchronized (C) {
                C.set(row, col, sum);
            }
        }
    }
}
