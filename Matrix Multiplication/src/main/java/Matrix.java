public class Matrix {
    private int ncols;
    private int nrows;
    private float _data[];

    public Matrix(int r, int c) {
        this.ncols = c;
        this.nrows = r;
        _data = new float[c*r];
    }

    public float[] get_data() {
        return _data;
    }

    public float get(int r, int c) {
        return _data[r*ncols + c];
    }

    public void set(int r, int c, float v) {
        _data[r*ncols +c] = v;
    }

    public int rows() {
        return nrows;
    }

    public int cols() {
        return ncols;
    }
}
