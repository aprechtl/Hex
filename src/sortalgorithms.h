#ifndef SORTALGORITHMS_H
#define SORTALGORITHMS_H

template <class T>
void insertionSort(T *array, int numElements, bool (*lessThanComp)(const T&, const T&))
{
    for (int j = 1; j < numElements; ++j)
    {
        T key(array[j]);
        int i = j - 1;

        while (i >= 0 && !lessThanComp(array[i], key))
        {
            array[i + 1] = array[i];
            --i;
        }

        array[i + 1] = key;
    }
}

template <class T> // returns a new, merged array; arrays must be pre-sorted
T *mergeSort(T *firstArray, int firstArrayCount, T *secondArray, int secondArrayCount, bool (*lessThanComp)(const T&, const T&))
{
    T *newArray = new T[firstArrayCount + secondArrayCount];

    int firstArrayCounter = 0;
    int secondArrayCounter = 0;
    int newArrayCounter = 0;

    while (firstArrayCounter < firstArrayCount && secondArrayCounter < secondArrayCount)
    {
        if (lessThanComp(firstArray[firstArrayCounter], secondArray[secondArrayCounter]))
        {
            newArray[newArrayCounter] = firstArray[firstArrayCounter];
            ++firstArrayCounter;
        }
        else
        {
            newArray[newArrayCounter] = secondArray[secondArrayCounter];
            ++secondArrayCounter;
        }

        ++newArrayCounter;
    }

    // move remaining elements to end of new array when one merging array is empty
    while (firstArrayCounter < firstArrayCount)
    {
        newArray[newArrayCounter] = firstArray[firstArrayCounter];
        ++firstArrayCounter;
        ++newArrayCounter;
    }

    while (secondArrayCounter < secondArrayCount)
    {
        newArray[newArrayCounter] = secondArray[secondArrayCounter];
        ++secondArrayCounter;
        ++newArrayCounter;
    }

    return newArray;
}

#endif
