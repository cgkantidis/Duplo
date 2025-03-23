#ifndef MATRIX_H
#define MATRIX_H

//#define BYTE_MATRIX

#include <memory>
#include <stdexcept>
#include <vector>

class Matrix {
private:
    std::size_t m_rows;
    std::size_t m_cols;
    std::size_t m_max_size;
#ifndef BYTE_MATRIX
    std::vector<bool> m_matrix;
#else
    std::unique_ptr<bool[]> m_matrix;
#endif

public:
    Matrix(std::size_t rows, std::size_t cols)
        : m_rows(rows), m_cols(cols), m_max_size(rows * cols),
#ifndef BYTE_MATRIX
          m_matrix(rows * cols) {}
#else
          m_matrix(std::make_unique<bool[]>(rows * cols)) {}
#endif

    [[nodiscard]] std::pair<std::size_t, std::size_t> shape() const {
        return { m_rows, m_cols };
    }

    void reshape(std::size_t rows, std::size_t cols) {
        if (rows * cols > m_max_size) {
            throw std::runtime_error("The Matrix cannot be reshaped to the specified shape because it would excceed its size");
        }
        m_rows = rows;
        m_cols = cols;
#ifndef BYTE_MATRIX
        std::fill_n(std::begin(m_matrix), rows * cols, false);
#else
        std::fill_n(m_matrix.get(), rows * cols, false);
#endif
    }

    bool operator()(std::size_t row, std::size_t col) const {
        return m_matrix[row * m_cols + col];
    }
    void operator()(std::size_t row, std::size_t col, bool value) {
        m_matrix[row * m_cols + col] = value;
    }
    static std::size_t max_size() {
        return std::vector<bool>().max_size();
    }
};

#endif
