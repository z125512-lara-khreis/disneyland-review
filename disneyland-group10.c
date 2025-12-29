#include <stdio.h>

int main()
{
    int choice;

    printf("****** Welcome to our Disneyland Reviewing System! ******\n\n");
    printf("Please choose one option to continue by entering a number:\n\n");

    char *options[] = {"1 Display Reviews", "2 Add Review", "3 Delete Review", "4 Edit Review", "5 Exit"};
    int numberOfStrings = sizeof(options) / sizeof(options[0]);
    
    for (int i = 0; i < numberOfStrings; i++)
    {
        printf("%s", options[i]);

        if (i == numberOfStrings - 1) // If last element of array...
        {
            printf("\n\n");          // ... add two escape sequences
        } else
        {
            printf("\n");
        }
    }
    
    printf("Option: ");
    scanf("%d", &choice);

    return 0;
}
