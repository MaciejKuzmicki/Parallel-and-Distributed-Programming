import java.io.*;
import java.util.Locale;
import java.util.Scanner;

public class MatrixMult {

    public static void main(String[] args) {
        MatrixMult mm = new MatrixMult();

        try {
            mm.start(args);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }

    private double frobeniusNorm(Matrix A) {
        double frobeniusNorm = 0;
        for(int i = 0; i < A.rows()*A.cols(); i++) {
            frobeniusNorm += A.get_data()[i]*A.get_data()[i];
        }
        return Math.sqrt(frobeniusNorm);
    }

    protected void start(String[] args) throws FileNotFoundException {
        Matrix A,B;
        A = read(args[0]);
        B = read(args[1]);
        int numberOfThreads = Integer.parseInt(args[2]);
        if(numberOfThreads <= 0) {
            throw new IllegalArgumentException("Number of threads should be higher than 0");
        }

        System.out.println("Matrix A: ");
        print(A);

        System.out.println("\nMatrix B:");
        print(B);
        if(A.cols() != B.rows()) {
            throw new IllegalArgumentException("Matrices cannot be multiplied");
        }
        Matrix C = mult(A,B, numberOfThreads);
        System.out.println("A*B = ");

        print(C);
        double frobeniusNorm = frobeniusNorm(C);
        System.out.println("Frobenius norm: " + frobeniusNorm);

    }

    private Matrix mult(Matrix A, Matrix B, int n)  {
        Matrix C = new Matrix(A.rows(), B.cols());
        int numberOfCellsInFinalMatrix = A.rows()*B.cols();
        if(n > numberOfCellsInFinalMatrix) {
            System.out.println("Number of threads is higher than the number of operations to make, so i am gonna use " + numberOfCellsInFinalMatrix + " threads");
            n = numberOfCellsInFinalMatrix;
        }
        Thread[] threads = new Thread[n];
        int numberOfThreadsThatHaveMoreCellsToCalculate = numberOfCellsInFinalMatrix % n;
        int cellsPerThread = numberOfCellsInFinalMatrix / n;

        for (int i = 0; i < n; i++) {
            int startCellIndex = i * cellsPerThread + Math.min(i, numberOfThreadsThatHaveMoreCellsToCalculate);
            int endCellIndex = startCellIndex + cellsPerThread - 1 + (i < numberOfThreadsThatHaveMoreCellsToCalculate ? 1 : 0);
            threads[i] = new Thread(new MatrixThreadMultiplier(A, B, C, startCellIndex, endCellIndex));
            threads[i].start();
        }

        try {
            for (Thread thread : threads) {
                thread.join();
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        return C;
    }



    protected Matrix read(String fname) throws FileNotFoundException {
        File f = new File(fname);
        Scanner scanner = new Scanner(f).useLocale(Locale.US);

        int rows = scanner.nextInt();
        int cols = scanner.nextInt();

        scanner.nextLine();

        Matrix res = new Matrix(rows, cols);

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                res.set(r, c, scanner.nextFloat());
            }
        }

        return res;
    }

    protected void print(Matrix m) {
        System.out.println("[");
        for (int r = 0; r < m.rows(); r++) {

            for (int c = 0; c < m.cols(); c++) {
                System.out.print(m.get(r,c));
                System.out.print(" ");
            }

            System.out.println("");
        }
        System.out.println("]");
    }




}
