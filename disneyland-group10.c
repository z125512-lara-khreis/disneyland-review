#include <stdio.h>
#include <string.h>

#define MAX 100
#define LINE 1024
#define REVIEW_LEN 4000

/* =========================
Struct for collect Review
========================= */
struct Review {
    int id;                     // Review ID
    int rating;                 // Rating
    char month[20];             // Review month
    char location[50];          // Reviewer location
    char review[REVIEW_LEN];    // Review text 
    char branch[50];            // Disneyland branch
};

/* =========================
   Global variables
========================= */
struct Review reviews[MAX];     // array collect all review 
int count = 0;                  // amount of review 

//function loadcsv
void loadCSV()
{
    FILE *fp = fopen("disneylandreview.csv", "r");

    char line[LINE];
    char buffer[REVIEW_LEN];

    /* skip header */
    fgets(line, LINE, fp);

    /* read data */
    while (fgets(line, LINE, fp) && count < MAX) {

        buffer[0] = '\0';

        /* if review have " maybe it have many line */
        if (strchr(line, '"')) {
            strcat(buffer, line);

            /* read until find " again */
            while (!strchr(buffer + 1, '"')) {
                if (!fgets(line, LINE, fp))
                    break;
                strcat(buffer, line);
            }
        } else {
            strcpy(buffer, line);
        }

        sscanf(buffer,
            "%d,%d,%19[^,],%49[^,],\"%3999[^\"]\",%49[^\n]",
            &reviews[count].id,
            &reviews[count].rating,
            reviews[count].month,
            reviews[count].location,
            reviews[count].review,
            reviews[count].branch
        );

        count++;
    }

    fclose(fp);
}

// save function
void saveCSV()
{
    FILE *fp = fopen("disneylandreview.csv", "w");

    /* write header */
    fprintf(fp,
        "Review_ID,Rating,Review_Month,Reviewer_Location,Review_Text,Branch\n");

    /* write review in file csv */
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d,%d,%s,%s,\"%s\",%s\n",
            reviews[i].id,
            reviews[i].rating,
            reviews[i].month,
            reviews[i].location,
            reviews[i].review,
            reviews[i].branch
        );
    }

    fclose(fp);
}

// find data by ID
int findByID(int id)
{
    for (int i = 0; i < count; i++) {
        if (reviews[i].id == id)
            return i;   // เจอ → ส่ง index กลับ
    }
    return -1;          // ไม่เจอ
}

// function editReview
void editReview(int index)
{
    printf("\n--- Edit Review ---\n");

    printf("Enter the Rating: ");
    scanf("%d", &reviews[index].rating);

    printf("Enter the Month: ");
    scanf(" %19[^\n]", reviews[index].month);

    printf("Enter the Location: ");
    scanf(" %49[^\n]", reviews[index].location);

    printf("Enter the Review: ");
    scanf(" %3999[^\n]", reviews[index].review);

    printf("Enter the Branch: ");
    scanf(" %49[^\n]", reviews[index].branch);

    //save file
    saveCSV();

    printf("\nReview updated successfully!\n");
}

//funtion show all process
void editMenu()
{
    loadCSV();   // use function loadcsv

    //Giving user Enter review id
    int id;
    printf("Enter review ID: ");
    scanf("%d", &id);

    int index = findByID(id);
    if (index == -1) {
        printf("Review ID not found\n");
        return;
    }

    /* show data to make sure!! */
    printf("\nID: %d\n", reviews[index].id);
    printf("Rating: %d\n", reviews[index].rating);
    printf("Month: %s\n", reviews[index].month);
    printf("Location: %s\n", reviews[index].location);
    printf("Review: %s\n", reviews[index].review);
    printf("Branch: %s\n", reviews[index].branch);

    char confirm;
    printf("\nEdit this review? (y/n): ");
    scanf(" %c", &confirm);

    if (confirm == 'y' || confirm == 'Y') {
        editReview(index);
    }
}

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
            printf("\n\n"); // ... add two escape sequences
        }
        else
        {
            printf("\n");
        }
    }

    printf("Option: ");
    scanf("%d", &choice);

    switch (choice)
    {
    case 1:
        /* displayReviews(); */
        break;

    case 2:
        /* addReview(); */
        break;

    case 3:
        /* deleteReview(); */
        break;

    case 4:
        editMenu(); // Call Edit function
        break;

    case 5:
        printf("Exit!\n");
        return 0;

    default:
        printf("Invalid option\n");
    }

    return 0;
}
