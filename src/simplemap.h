#ifndef SIMPLEMAP_H
#define SIMPLEMAP_H

template <class Key, class Value>
class SimpleMap
{
private:
    Key *keyArray;
    Value *valueArray;
    int size;

public:
    // constructs empty map
    SimpleMap() : keyArray(0), valueArray(0), size(0)
    {
    }

    // constructs map and takes ownership of the arrays, which must be sorted beforehand
    SimpleMap(Key *keys, Value *values, int count)
        : keyArray(keys), valueArray(values), size(count)
    {
    }

    SimpleMap(const SimpleMap<Key, Value> &other)
        : size(other.size)
    {
        if (size == 0)
        {
            keyArray = 0;
            valueArray = 0;
            return;
        }

        keyArray = new Key[size];
        valueArray = new Value[size];

        for (int i = 0; i < size; ++i)
        {
            keyArray[i] = other.keyArray[i];
            valueArray[i] = other.valueArray[i];
        }
    }

    ~SimpleMap()
    {
        delete [] keyArray;
        delete [] valueArray;

        keyArray = 0;
        valueArray = 0;
    }

    // #######################################################################
    // ######################################################### CONST METHODS

    bool contains(Key key) const
    {
        if (size == 0)
            return false;

        if (key < keyArray[0] || key > keyArray[size - 1])
            return false;

        for (int i = 0; i < size; ++i)
        {
            if (keyArray[i] == key)
                return true;
        }

        return false;
    }

    int count() const
    {
        return size;
    }

    int find(Key key) const
    {
        for (int i = 0; i < size; ++i)
        {
            if (keyArray[i] == key)
                return i;
        }

        return -1; // if key not found
    }

    const Key *getKeyArray() const
    { return keyArray; }

    const Value *getValueArray() const
    { return valueArray; }

    const Key keyAt(int index) const
    { return keyArray[index]; }

    const Value valueAt(int index) const
    { return valueArray[index]; }

    // #######################################################################
    // ############################################# INSERT AND REMOVE METHODS

    void insert(Key key, Value value)
    {
        int newSize = size + 1;
        Key *keyArrayCopier = new Key[newSize];
        Value *valueArrayCopier = new Value[newSize];

        int i = 0;

        // ========================= COPY ARRAY CONTENT BEFORE INSERTION POINT
        while (i < size)
        {
            if (key < keyArray[i])
                break;

            keyArrayCopier[i] = keyArray[i];
            valueArrayCopier[i] = valueArray[i];

            ++i;
        }
        // ===================================================================

        keyArrayCopier[i] = key;
        valueArrayCopier[i] = value;

        // ========================== COPY ARRAY CONTENT AFTER INSERTION POINT
        while (i < size) {
            int iPlusOne = i + 1;
            keyArrayCopier[iPlusOne] = keyArray[i];
            valueArrayCopier[iPlusOne] = valueArray[i];

            ++i;
        }
        // ===================================================================

        delete [] keyArray;
        delete [] valueArray;

        size = newSize;
        keyArray = keyArrayCopier;
        valueArray = valueArrayCopier;
    }

    void remove(Key key)
    {
        int indexOfKeyToBeRemoved = find(key);

        if (indexOfKeyToBeRemoved == -1) // key not found
            return;

        int newSize = size - 1;

        if (newSize == 0)
        {
            delete [] keyArray;
            delete [] valueArray;

            size = 0;
            keyArray = 0;
            valueArray = 0;
            return;
        }

        Key *newKeyArray = new Key[newSize];
        Value *newValueArray = new Value[newSize];

        for (int i = 0; i < indexOfKeyToBeRemoved; ++i)
        {
            newKeyArray[i] = keyArray[i];
            newValueArray[i] = valueArray[i];
        }

        for (int i = indexOfKeyToBeRemoved; i < newSize; ++i)
        {
            int iPlusOne = i + 1;
            newKeyArray[i] = keyArray[iPlusOne];
            newValueArray[i] = valueArray[iPlusOne];
        }

        delete [] keyArray;
        delete [] valueArray;

        size = newSize;
        keyArray = newKeyArray;
        valueArray = newValueArray;
    }

    // #######################################################################
    // ####################################################### REPLACE METHODS

    void replaceKey(Key oldKey, Key newKey)
    { keyArray[find(oldKey)] = newKey; }

    void replaceKeyAt(int index, Key key)
    { keyArray[index] = key; }

    void replaceKeyAndValue(Key oldKey, Key newKey, Value newValue)
    {
        int i = find(oldKey);
        keyArray[i] = newKey;
        valueArray[i] = newValue;
    }

    void replaceKeyAndValueAt(int index, Key newKey, Value newValue)
    {
        keyArray[index] = newKey;
        valueArray[index] = newValue;
    }

    void replaceValue(Key key, Value value)
    { valueArray[find(key)] = value; }

    void replaceValueAt(int index, Value value)
    { valueArray[index] = value; }

    // #######################################################################
    // ################################################### ASSIGNMENT OPERATOR

    SimpleMap<Key, Value>& operator =(const SimpleMap<Key, Value> &other)
    {
        delete [] keyArray;
        delete [] valueArray;

        size = other.size;

        if (size == 0)
        {
            keyArray = 0;
            valueArray = 0;
            return *this;
        }

        keyArray = new Key[size];
        valueArray = new Value[size];

        for (int i = 0; i < size; ++i)
        {
            keyArray[i] = other.keyArray[i];
            valueArray[i] = other.valueArray[i];
        }

        return *this;
    }
};

#endif
