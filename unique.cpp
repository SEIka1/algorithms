#include <iostream>
#include <cstring>

char nonUniqueString[100];
char uniqueString[100];
char* getUniqueString(char* unqieString, const char* nonUniqueString)
{   
    int k = 1;
    int counterOfArray = 0;
    uniqueString[0] = nonUniqueString[0];
    for (int i = 0; i < strlen(nonUniqueString); i++)
    {
        for (int j = 0; j < strlen(uniqueString); j++)
        {
            if (nonUniqueString[i] == uniqueString[j]) 
               counterOfArray++;
        }
        if (counterOfArray == 0) {
            uniqueString[k] = nonUniqueString[i];
            k++;
        }

        counterOfArray = 0;
        
    }
    return uniqueString;
}
    int main(){
        int errcounter = 0;
    std::cin >> nonUniqueString;
    for (int i = 0; i < strlen(nonUniqueString); i++)
        for (int j = 0; j < strlen(nonUniqueString); j++)
            if (nonUniqueString[i] == nonUniqueString[j])
                errcounter++;
    if ((errcounter/strlen(nonUniqueString)) == strlen(nonUniqueString))
        std::cerr << "No unique symbols";
    else {
        try {
            std::cout << getUniqueString(uniqueString, nonUniqueString);
        }
        catch (std::invalid_argument)
        {
            throw "Invalid value: ";
        }
    }
    return 0;
}
