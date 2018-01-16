#ifndef SIMPLEVECTOR_H
#define SIMPLEVECTOR_H

#define ENABLE_SUPPORT_FOR_QT

#ifdef ENABLE_SUPPORT_FOR_QT
#include <QtCore/QList>
#endif

template <class T>
class SimpleVector
{
private:
    int m_size;
    int m_capacity;
    T *m_array;

// ###########################################################################
// ################################## CONSTRUCTORS, DESTRUCTOR, AND DELETE ALL

public:
    SimpleVector() : m_size(0), m_capacity(0), m_array(0)
    {
    }

    SimpleVector(int capacity)
        : m_size(0), m_capacity(capacity), m_array((capacity > 0) ? new T[capacity] : 0)
    {
    }

    SimpleVector(const SimpleVector<T> &a)
        : m_size(a.size()), m_capacity(a.capacity()), m_array((a.capacity() > 0) ? new T[a.capacity()] : 0)
    {
        for (int i = 0; i < m_size; ++i)
        {
            m_array[i] = a[i];
        }
    }

    SimpleVector(T *array, int elementCount)
        : m_size(elementCount), m_capacity(elementCount), m_array(array)
    {
    }

#ifdef ENABLE_SUPPORT_FOR_QT
    SimpleVector(const QList<T> &a)
        : m_size(a.size()), m_capacity(m_size), m_array((m_size > 0) ? new T[m_size] : 0)
    {
        for (int i = 0; i < m_size; ++i)
        {
            m_array[i] = a.at(i);
        }
    }
#endif // ENABLE_SUPPORT_FOR_QT

    ~SimpleVector()
    {
        delete [] m_array;
        m_array = 0;
    }

    // #######################################################################
    // ######################################################## ACCESS METHODS

    T& operator[](int index) const // no bounds checking
    {
        return m_array[index];
    }

    T& at(int index) const // no bounds checking
    {
        return m_array[index];
    }

    // #######################################################################
    // #################################### APPEND, INSERT, AND REMOVE METHODS

    void append(const T &obj) // does not reallocate memory
    {
        m_array[m_size] = obj;
        ++m_size;
    }

    void appendByValue(T obj) // does not reallocate memory
    {
        m_array[m_size] = obj;
        ++m_size;
    }

    void appendSafely(const T &obj) // reallocates memory if necessary
    {
        if (m_size == m_capacity)
        {
            ++m_capacity;
            T *newArray = new T[m_capacity];

            for (int i = 0; i < m_size; ++i)
            {
                newArray[i] = m_array[i];
            }

            delete [] m_array;
            m_array = newArray;
        }

        m_array[m_size] = obj;
        ++m_size;
    }

    void insertSafely(int index, const T &obj) // reallocates memory if necessary
    {
        if (m_capacity <= m_size)
            ++m_capacity;

        T *newArray = new T[m_capacity];

        for (int i = 0; i < index; ++i)
        {
            newArray[i] = m_array[i];
        }

        newArray[index] = obj;

        for (int i = index; i < m_size; ++i)
        {
            newArray[i + 1] = m_array[i];
        }

        delete [] m_array;
        m_array = newArray;
        ++m_size;
    }

    void removeIndex(int position) // does not check bounds or reallocate memory
    {
        --m_size;

        for (int i = position; i < m_size; ++i)
        {
            m_array[i] = m_array[i + 1];
        }
    }

    void removeValue(const T &value) // does not reallocate memory
    {
        int index = find(value);

        if (index == -1)
            return;

        removeIndex(index);
    }

    // #######################################################################
    // #################################################### MANUAL SIZE SETTER
    // sets size so that you can use append() to overwrite existing data
    // without having to reset the memory
    void setSize(int size = 0)
    {
        m_size = size;
    }

    // #######################################################################
    // ############################################################### GETTERS

    int capacity() const
    {
        return m_capacity;
    }

    int size() const
    {
        return m_size;
    }

    // #######################################################################
    // ########################################################### INFORMATION

    bool contains(const T &obj) const
    {
        return (find(obj) >= 0);
    }

    int find(const T &obj) const
    {
        for (int i = 0; i < m_size; ++i)
        {
            if (obj == m_array[i])
                return i;
        }

        return -1; // not found
    }

    // #######################################################################
    // ################################################### ASSIGNMENT OPERATOR

    SimpleVector<T>& operator=(const SimpleVector<T> &other)
    {
        delete [] m_array;

        m_size = other.m_size;
        m_capacity = other.m_capacity;

        if (m_capacity == 0)
        {
            m_array = 0;
        }
        else
        {
            m_array = new T[m_capacity];

            for (int i = 0; i < m_size; ++i)
            {
                m_array[i] = other[i];
            }
        }

        return *this;
    }
};

#endif
