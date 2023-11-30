#ifndef COMPARISONS_H
#define COMPARISONS_H

bool lessThan() {
    if (a!=b) {
        return a < b;
    }
    const bool greaterThan = aluFlags & 1;
    const bool equals = aluFlags & 2;
    if(equals) return false;
    if(greaterThan) return false;
    return true;
}

bool greaterOrEqualThan() {
    return !lessThan();
}

bool equalThan() {
    if(a != b) {
        return false;
    }
    return aluFlags & 2; // Equals
}

bool lessOrEqualThan() {
    return lessThan() || equalThan();
}

bool notEqualThan() {
    return !equalThan();
}

bool greaterThan () {
    return !lessOrEqualThan();
}

#endif
