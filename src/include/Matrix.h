#ifndef MATRIX_H
#define MATRIX_H

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#define BYTE_MATRIX
#ifndef BYTE_MATRIX
class Matrix {
private:
    std::size_t m_rows{};
    std::size_t m_cols{};
    std::size_t m_max_size{};
    std::vector<bool> m_matrix;
public:
    Matrix(std::size_t rows, std::size_t cols)
        : m_rows(rows), m_cols(cols), m_max_size(rows * cols),
          m_matrix(rows * cols) {}

    [[nodiscard]] std::pair<std::size_t, std::size_t> shape() const {
        return { m_rows, m_cols };
    }

    void reshape(std::size_t rows, std::size_t cols) {
        if (rows * cols > m_max_size) {
            throw std::runtime_error("The Matrix cannot be reshaped to the specified shape because it would excceed its size");
        }
        m_rows = rows;
        m_cols = cols;
        std::fill_n(std::begin(m_matrix), rows * cols, false);
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

#else

class Matrix {
private:
    std::size_t m_rows{};
    std::size_t m_cols{};
    std::size_t m_max_size{};
    bool *m_matrix{};

    void swap(Matrix &other) noexcept {
        std::swap(m_rows, other.m_rows);
        std::swap(m_cols, other.m_cols);
        std::swap(m_max_size, other.m_max_size);
        std::swap(m_matrix, other.m_matrix);
    }

public:
    Matrix(std::size_t rows, std::size_t cols)
        : m_rows(rows), m_cols(cols), m_max_size(rows * cols),
          m_matrix(new bool[rows * cols]) {}
    Matrix(Matrix &) = delete;
    Matrix &operator=(Matrix &) = delete;
    Matrix(Matrix&& other) noexcept {
        swap(other);
    }
    Matrix &operator=(Matrix&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        swap(other);
        return *this;
    }
    ~Matrix() {
        delete[] m_matrix;
    }

    [[nodiscard]] std::pair<std::size_t, std::size_t> shape() const {
        return { m_rows, m_cols };
    }

    void reshape(std::size_t rows, std::size_t cols) {
        if (rows * cols > m_max_size) {
            throw std::runtime_error("The Matrix cannot be reshaped to the specified shape because it would excceed its size");
        }
        m_rows = rows;
        m_cols = cols;
        std::fill_n(m_matrix, rows * cols, false);
    }

    bool operator()(std::size_t row, std::size_t col) const {
        return m_matrix[row * m_cols + col];
    }

    void operator()(std::size_t row, std::size_t col, bool value) {
        m_matrix[row * m_cols + col] = value;
    }

    static std::size_t max_size() {
        return std::vector<std::byte>().max_size();
    }
};
#endif // BYTE_MATRIX
#endif
