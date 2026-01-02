#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// ---Add Options ---//
/*Writes one text field into the CSV. It puts the text in quotes. It doubles any " inside the text. */
void write_csv_field(FILE *dl, char text[])
{
    int i = 0;
    fprintf(dl, "\"");

    while (text[i] != '\0')
    {
        if (text[i] == '"')
            fprintf(dl, "\"");

        fprintf(dl, "%c", text[i]);
        i++;
    }

    fprintf(dl, "\"");
}

/*Checks if the file exists. It tries to open the file in read mode. It returns 1 if it works, otherwise 0.*/
int file_exists(const char *filename)
{
    FILE *dl = fopen(filename, "r");
    if (dl == NULL)
        return 0;

    fclose(dl);
    return 1;
}

/*Finds the next Review_ID. It reads the CSV line by line. It returns the biggest ID + 1.*/
int get_next_id(const char *filename)
{
    FILE *dl = fopen(filename, "r");
    char line[2000];
    int id;
    int last_id = 0;

    if (dl == NULL)
        return 1;

    while (fgets(line, 2000, dl) != NULL)
    {
        if (sscanf(line, "%d,", &id) == 1)
            last_id = id;
    }

    fclose(dl);
    return last_id + 1;
}

/*Asks the user for all review data. It appends the new review at the bottom of the CSV.*/
void add_review_append_only(const char *filename)
{
    int existed = file_exists(filename);
    int next_id = get_next_id(filename);

    int rating;
    char month[100];
    char location[200];
    char review_text[2000];
    char branch[200];

    printf("Rating (1-5): ");
    scanf("%d", &rating);
    getchar();

    printf("Review_Month (z.B. April): ");
    scanf(" %99[^\n]", month);
    getchar();

    printf("Reviewer_Location: ");
    scanf(" %199[^\n]", location);
    getchar();

    printf("Review_Text: ");
    scanf(" %1999[^\n]", review_text);
    getchar();

    printf("Branch (e.g. Disneyland_Paris): ");
    scanf(" %199[^\n]", branch);
    getchar();

    FILE *dl = fopen(filename, "a");
    if (dl == NULL)
    {
        printf("Error: File not found.\n");
        return;
    }

    if (!existed)
    {
        fprintf(dl, "Review_ID,Rating,Review_Month,Reviewer_Location,Review_Text,Branch\n");
    }

    fprintf(dl, "%d,%d,", next_id, rating);

    write_csv_field(dl, month);
    fputc(',', dl);
    write_csv_field(dl, location);
    fputc(',', dl);
    write_csv_field(dl, review_text);
    fputc(',', dl);
    write_csv_field(dl, branch);
    fputc('\n', dl);

    fclose(dl);

    printf("Thank you! We have successfully received your review.\n");
}
// --- Add Options ---//

// --- Delete Options ---//
void delete_review(const char *filename)
{
    FILE *original = fopen(filename, "r");
    FILE *temp = fopen("temp.csv", "w");

    char line[2000];
    int delete_id;
    int current_id;
    int found = 0;

    if (original == NULL || temp == NULL)
    {
        printf("Error: File not found.\n");
        return;
    }

    printf("Enter the Review_ID to delete: ");
    scanf("%d", &delete_id);

    /* copy header line */
    if (fgets(line, sizeof(line), original) != NULL)
    {
        fprintf(temp, "%s", line);
    }

    /* read each review line */
    while (fgets(line, sizeof(line), original) != NULL)
    {
        if (sscanf(line, "%d,", &current_id) == 1)
        {
            if (current_id == delete_id)
            {
                found = 1;
                continue; // skip this line (delete)
            }
        }
        fprintf(temp, "%s", line);
    }

    fclose(original);
    fclose(temp);

    remove(filename);
    rename("temp.csv", filename);

    if (found)
        printf("Review deleted successfully.\n");
    else
        printf("Review_ID not found.\n");
}
//---Delete Options---//

// --- Edit Options ---//
#define MAX 100 // define max value of review
#define LINE 1024
#define REVIEW_LEN 4000

/* =========================
Struct for collect Review
========================= */
struct Review
{
    int id;                  // Review ID
    int rating;              // Rating
    char month[20];          // Review month
    char location[50];       // Reviewer location
    char review[REVIEW_LEN]; // Review text
    char branch[50];         // Disneyland branch
};

struct Review reviews[MAX]; // array collect all review
int count = 0;              // collect the amount of review

// function check int of id and rating
int inputInt(const char *message)
{
    char line[100];
    int value;

    while (1)
    {
        printf("%s", message);
        // if it not int it will be continue in the while loop
        if (!fgets(line, sizeof(line), stdin))
            continue;
        // condition if it "int" it will return the value
        if (sscanf(line, "%d", &value) == 1)
            return value;

        printf("Please enter numbers only.\n");
    }
}
// function loadcsv
void loadCSV()
{
    FILE *fp = fopen("disneylandreview.csv", "r");

    char line[LINE];
    char buffer[REVIEW_LEN];

    /* skip header */
    fgets(line, LINE, fp);

    /* read data */
    while (fgets(line, LINE, fp) && count < MAX)
    {

        buffer[0] = '\0';

        /* if it still don't have " it have many line because it the review sentence */
        if (strchr(line, '"'))
        {
            strcat(buffer, line);

            /* read until find " again */
            while (!strchr(buffer + 1, '"'))
            {
                if (!fgets(line, LINE, fp))
                    break;
                strcat(buffer, line);
            }
        }
        else
        {
            strcpy(buffer, line);
        }

        sscanf(buffer,
               "%d,%d,%19[^,],%49[^,],\"%3999[^\"]\",%49[^\n]",
               &reviews[count].id,
               &reviews[count].rating,
               reviews[count].month,
               reviews[count].location,
               reviews[count].review,
               reviews[count].branch);

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
    for (int i = 0; i < count; i++)
    {
        fprintf(fp, "%d,%d,%s,%s,\"%s\",%s\n",
                reviews[i].id,
                reviews[i].rating,
                reviews[i].month,
                reviews[i].location,
                reviews[i].review,
                reviews[i].branch);
    }

    fclose(fp);
}

// find data by ID
int findByID(int id)
{
    for (int i = 0; i < count; i++)
    {
        if (reviews[i].id == id)
            return i; // find → send index back
    }
    return -1; // can't find
}

// function editReview
void editReview(int index)
{
    printf("\n--- Edit Review ---\n");

    // use function inputint
    reviews[index].rating = inputInt("Enter the Rating: ");

    printf("Enter the Month: ");
    scanf(" %19[^\n]", reviews[index].month);

    printf("Enter the Location: ");
    scanf(" %49[^\n]", reviews[index].location);

    printf("Enter the Review: ");
    scanf(" %3999[^\n]", reviews[index].review);

    printf("Enter the Branch: ");
    scanf(" %49[^\n]", reviews[index].branch);

    // save file
    saveCSV();

    printf("\nReview updated successfully!\n");
}

// funtion show all process
void editMenu()
{
    loadCSV(); // use function loadcsv

    // Giving user to Enter review id
    //  use function inputint
    int id = inputInt("Enter Review ID: ");
    int index = findByID(id);

    if (index == -1)
    {
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
    printf("\nDo you want to edit this review? (y/n): ");
    scanf(" %c", &confirm);

    if (confirm == 'y' || confirm == 'Y')
    {
        editReview(index);
    }
}

// --- Edit Options ---//

/*Runs the programm*/
int main(void)
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
        add_review_append_only("disneylandreview.csv"); //Call Add function
        break;

    case 3:
        delete_review("disneylandreview.csv"); // Call Delete function
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

    /*get_next_id("disneylandreview.csv");
    add_review_append_only("disneylandreview.csv");*/

    // delete_review("disneylandreview.csv");

    return 0;
}
