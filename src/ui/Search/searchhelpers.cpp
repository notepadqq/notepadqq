#include "include/Search/searchhelpers.h"

std::vector<int> SearchHelpers::linePositions(const QString& data)
{
    const int dataSize = data.size();
    std::vector<int> linePosition;

    linePosition.push_back(0);

    // Finds line-breaks in the string. Doesn't check the last char so the loop
    // can be optimized.
    for (int i = 0; i < dataSize - 1; i++) {
        if (data[i] == '\r' && data[i + 1] == '\n') {
            linePosition.push_back(i + 2);
            i++;
        } else if (data[i] == '\r' || data[i] == '\n') {
            linePosition.push_back(i + 1);
        }
    }

    // Check the last char manually.
    if (dataSize > 0 && (data.back() == '\r' || data.back() == '\n'))
        linePosition.push_back(dataSize);

    linePosition.push_back(dataSize);
    return linePosition;
}
