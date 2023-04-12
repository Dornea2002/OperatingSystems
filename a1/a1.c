#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


int listingDirectors(const char *path)
{

    DIR* dir;
    dir = opendir(path); // open dir

    struct dirent *entry;

    // check if dir is opened
    if(dir == NULL)
    {
        printf("ERROR\ninvalid directory path\n");
        return -1;
    }
    else printf("SUCCESS\n");

    // read inputs
    while((entry = readdir(dir)) != NULL)
    {
        // check if file is valid
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            printf("%s/%s\n", path, entry->d_name);
        }
    }

    closedir(dir);

    return 0;
}

void listingDirectorsRecursive(char* path, int displayGood)
{
    DIR* dir;
    struct dirent *entry;
    struct stat statBuffer;
    char fullPath[512];

    dir=opendir(path);
    entry = NULL;

    // check if dir is opened
    if(dir == NULL)
    {
        printf("ERROR\ninvalid directory path\n");
        return;
    }

    else
    {
        if(displayGood == 1)
        {
            printf("SUCCESS\n");
            displayGood = 0;
        }
    }

    // read inputs
    while((entry = readdir(dir)) != NULL)
    {
        // check if files are ok
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            //build the full path
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);

            if(lstat(fullPath, &statBuffer) == 0)
            {
                printf("%s\n", fullPath);

                //if file is dir, then we continue
                if(S_ISDIR(statBuffer.st_mode))
                {
                    listingDirectorsRecursive(fullPath, displayGood);
                }
            }
        }
    }

    closedir(dir);
}

void listingDirectorsName(const char *path, const char *nameEndsWith)
{
    DIR *dir;
    struct dirent *entry;
    struct stat statBuffer;
    char fullPath[512];

    //open dir
    dir = opendir(path);
    entry = NULL;

    //check if dir is opened
    if(dir == NULL)
    {

        printf("ERROR\ninvalid directory path\n");
        return;

    }

    else printf("SUCCESS\n");

    // read inputs
    while((entry = readdir(dir)) != NULL)
    {
        // check if file is ok
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            //check if the file ends with NameEndsWith
            if(strstr(entry->d_name, nameEndsWith))
            {
                //build full path
                snprintf(fullPath, 512, "%s/%s", path, entry->d_name);

                if(lstat(fullPath, &statBuffer) == 0)
                {
                    printf("%s\n", fullPath);
                }
            }
        }
    }

    closedir(dir);
}




void listingDirectorsPermissions(const char *path, int numberOctal)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    struct stat statbuf;
    char filePath[512];

    dir = opendir(path);

    if(dir == NULL)
    {
        printf("ERROR\ninvalid directory path\n");
        return;
    }

    else printf("SUCCESS\n");



    while((entry = readdir(dir)) != NULL)
    {

        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(filePath, 512, "%s/%s", path, entry->d_name);
                if(lstat(filePath, &statbuf) == 0)
                {

                    char octalBuf[10];
                    char octalPerm[10];

                    sprintf(octalBuf, "%o", statbuf.st_mode);
                    sprintf(octalPerm, "%d", numberOctal);

                    if(strcmp(octalBuf+3, octalPerm)==0)
                    {
                        printf("%s\n", filePath);
                     }
                }
        }
    }
    closedir(dir);
}





int parsingFile(const char* path)
{
    //open file
    int file=open(path, O_RDONLY);

    // check if file is opened
    if(file == -1)
    {
        printf("Could not open the file!\n");
        return -1;
    }


    char magic[1];
    magic[1]='\0';

    lseek(file, -1, SEEK_END); //seek for magic
    read(file, magic, 1);

    if(strcmp(magic, "6") != 0) //if magic is valid
    {
        printf("ERROR\nwrong magic\n");
        return -1;
    }

    int  headerSize = 0, version = 0, numberSections = 0;

    lseek(file, -3, SEEK_END);
    read(file, &headerSize, 2);

    lseek(file, -headerSize, SEEK_END);
    read(file, &version, 4);

    if(!(version >= 109 && version <= 173)) // if version is valid
    {
        printf("ERROR\nwrong version\n");
        return -1;
    }

    lseek(file, -headerSize+4, SEEK_END);
    read(file, &numberSections, 1);

    if(!(numberSections >= 4 && numberSections <= 18)) // if numberSections is valid
    {
        printf("ERROR\nwrong sect_nr\n");
        return -1;
    }

    char name[7];
    int type = 0, size = 0, offset = 0;

    //read the fields to check if they are valid
    for(int i = 0; i < numberSections; i++)
    {
        read (file, name, 6);
        name[6] = '\0';
        read (file, &type, 2);
        read (file, &offset, 4);
        read (file, &size, 4);

        // if type is not valid, then error
        if(type != 27 && type != 25 && type !=  67 && type != 93)
        {
            printf("ERROR\nwrong sect_types\n");
            return -1;
        }
    }

    lseek(file, -headerSize+5, SEEK_END); //move the cursor to secton_headers

    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", numberSections);

    //display all
    for(int i = 0; i < numberSections; i++)
    {
        read(file, name, 6);
        name[6]='\0';
        read(file, &type, 2);
        read(file, &offset, 4);
        read(file, &size, 4);

        printf("section%d: %s %d %d\n", i + 1, name, type, size);
    }
    close(file);

    return 0;
}

int extractLine(const char* path, int section, int line)
{
    //open file
    int file=open(path, O_RDONLY);


    // check if file is opened
    if(file == -1)
    {
        printf("ERROR\ninvalid file\n");
        return -1;
    }

    int  headerSize = 0, numberSections=0;

    lseek(file, -3, SEEK_END);
    read(file, &headerSize, 2);

    lseek(file, -headerSize+4, SEEK_END);
    read(file, &numberSections, 1);

    if(!(numberSections >= 4 && numberSections <= 18)) // if numberSections is valid
    {
        return -1;
    }

    if(section>numberSections || section<=0){
        printf("ERROR\ninvalid section\n");
        return -1;
    }


    char name[7];
    int type = 0, size = 0, offset = 0;

    lseek(file, -headerSize+5, SEEK_END); //move the cursor to secton_headers

    printf("SUCCESS\n");

    //display all
    for(int i = 0; i < section; i++)
    {
        read(file, name, 6);
        name[6]='\0';
        read(file, &type, 2);
        read(file, &offset, 4);
        read(file, &size, 4);
    }
    // printf("section%d: %s %d %d\n", section, name, type, size);

    lseek(file, offset, SEEK_SET);

    int lineNumber=1;
    char *copy=(char*)malloc(size*sizeof(char));
    char *string=(char*)malloc(size*sizeof(char));
    read(file, string, size);
    strcpy(copy, string);

    // printf("%s\n\n\n", string);

    char *linePoint=string;
    char*lineString=(char*)malloc(size*sizeof(char));

    do{
        linePoint=strchr(linePoint, '\x0A');
        if(linePoint){
            lineNumber++;
            linePoint=linePoint+1;
            // strcpy(linePoint, linePoint+1);
        }
    }while(linePoint!=NULL);

    // printf("Line number %d", lineNumber);

    int count=lineNumber-line;

    // printf("\n\n%d\n", count);
    while(count>0){
        linePoint=strstr(string, "\n");
        strcpy(string, linePoint+1);
        count--;
    }

    //  printf("\n\n%d\n", count);

    if(line==1){
        linePoint=strstr(string, "\n");
    }
    else linePoint=strstr(string, "\n")+1;
    // printf("%s\n", linePoint);
    strncpy(lineString, string, linePoint-string);
    printf("%s", lineString);


    free(string);
    free(copy);
    free(lineString);

    close(file);

	return 0;
}

int findTest(const char* path)
{
    //open file
    int file=open(path, O_RDONLY);

    // check if file is opened
    if(file == -1)
    {
        return -1;
    }


    char magic[1];
    magic[1]='\0';

    lseek(file, -1, SEEK_END); //seek for magic
    read(file, magic, 1);

    if(strcmp(magic, "6") != 0) //if magic is valid
    {
        return -1;
    }

    int  headerSize = 0, version = 0, numberSections = 0;

    lseek(file, -3, SEEK_END);
    read(file, &headerSize, 2);

    lseek(file, -headerSize, SEEK_END);
    read(file, &version, 4);

    if(!(version >= 109 && version <= 173)) // if version is valid
    {
        return -1;
    }

    lseek(file, -headerSize+4, SEEK_END);
    read(file, &numberSections, 1);

    if(!(numberSections >= 4 && numberSections <= 18)) // if numberSections is valid
    {
        return -1;
    }

    char name[7];
    int type = 0, size = 0, offset = 0;

    //read the fields to check if they are valid
    for(int i = 0; i < numberSections; i++)
    {
        read (file, name, 6);
        name[6] = '\0';
        read (file, &type, 2);
        read (file, &offset, 4);
        read (file, &size, 4);

        // if type is not valid, then error
        if(type != 27 && type != 25 && type !=  67 && type != 93)
        {
            return -1;
        }
    }

    lseek(file, -headerSize+5, SEEK_END); //move the cursor to secton_headers

    //display all
    int count=0;
    for(int i = 0; i < numberSections; i++)
    {
        read(file, name, 6);
        name[6]='\0';
        read(file, &type, 2);
        read(file, &offset, 4);
        read(file, &size, 4);
        if(type==25){
            count++;
        }

    }

    close(file);

    if(count<2) {
        return -1;
    }

    return 0;
}


void findAllFile(char* path, int allGood)
{

    DIR* dir;
    struct dirent *entry;
    struct stat statBufffer;
    char fullPath[2048];

    dir=opendir(path); //open dir
    entry = NULL;

    // check if dir is valid
    if(dir == NULL)
    {
        printf("ERROR\ninvalid directory path\n");
        return;
    }

    else
    {

	if(allGood == 1)
        {
            allGood = 0;
            printf("SUCCESS\n");
        }
    }
    // read the inputs
    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            //build full path
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);

            if(lstat(fullPath, &statBufffer) == 0)
            {
		        // check if file has at least 2 sections with type equal to 25
		        if(findTest(fullPath) == 0){
                    printf("%s\n", fullPath);
                }

                if(S_ISDIR(statBufffer.st_mode))
                {
                    findAllFile(fullPath, allGood);
                }
            }
        }
    }

    closedir(dir);
}

int main(int argc, char **argv){
    if(argc >= 2){

        if(strcmp(argv[1], "variant") == 0){
            printf("30299\n");
            return 0;
        }

        int simpleList = 0;

        //listing simple
        if(strcmp(argv[1], "list") == 0 && strstr(argv[2], "path=") && argc < 4)
        {
            if(strstr(argv[2], "path=") && argc < 4) // if we have list path=
            {
                int pathLength = strlen(argv[2]) - 5; //path= has 5 letters
                char* path = (char*)malloc(pathLength * sizeof(char)+1);


                strcpy(path, argv[2] + 5); //copy path syntax

                listingDirectors(path);
                simpleList = 1;
                free(path);
            }
        }

         //listing recursive
        if(simpleList == 0 && argc > 3)
        {
            if(strcmp(argv[3], "recursive") == 0)
            {
                if(strstr(argv[2], "path=")) // if we have list path= recursive
                {
                    int pathLength = strlen(argv[2]) - 5; //path= has 5 letters
                    char* path = (char*)malloc(pathLength * sizeof(char));


                    strcpy(path, argv[2] + 5); //copy path syntax

                    listingDirectorsRecursive(path, 1);
                    free(path);
                }
            }

            else if(strcmp(argv[2], "recursive") == 0)
            {
                if(strstr(argv[3], "path=")) //if we have list recursive path=
                {
                    int pathLength = strlen(argv[3]) - 5; //path= has 5 letters
                    char* path = (char*)malloc(pathLength * sizeof(char));


                    strcpy(path, argv[3] + 5); //copy path syntax
                    listingDirectorsRecursive(argv[3] + 5, 1);
                    free(path);
                }
            }
        }

        //list name ends with
        if(strstr(argv[2], "name_ends_with=") && strstr(argv[3], "path=")){//if we have list name_ends_with= path=
            int nameLength=strlen(argv[2])-15; //name_ends_with= has 15 letters
            char *nameEndsWith=(char*)malloc(nameLength*sizeof(char));

            strcpy(nameEndsWith, argv[2]+15); //copy name syntax

            int pathLength = strlen(argv[3]) - 5; //path= has 5 letters
            char* path = (char*)malloc(pathLength * sizeof(char));


            strcpy(path, argv[3] + 5); //copy path syntax

            listingDirectorsName(path, nameEndsWith);

            free(nameEndsWith);
            free(path);
        }

                //list permissions
        if(strstr(argv[2], "permissions=") && strstr(argv[3], "path=")){ //if we have permissions= path=

            int permissionsLength=strlen(argv[2])-12; //permissions= has 12 letters
            char *permissions=(char*)malloc(permissionsLength*sizeof(char));

            strcpy(permissions, argv[2]+12); //copy permissions syntax

            int pathLength = strlen(argv[3]) - 5; //path= has 5 letters
            char* path = (char*)malloc(pathLength * sizeof(char));


            strcpy(path, argv[3] + 5); //copy path syntax

            int numberOctal=0;
            int aux=0;

            if(permissions[0]=='r'){
                aux+=4;
            }
            else aux+=0;
            if(permissions[1]=='w'){
                aux+=2;
            }else aux+=0;
            if(permissions[2]=='x'){
                aux+=1;
            }else aux+=0;

            numberOctal=numberOctal*10+aux;
            aux=0;

            if(permissions[3]=='r'){
                aux+=4;
            }
            else aux+=0;
            if(permissions[4]=='w'){
                aux+=2;
            }else aux+=0;
            if(permissions[5]=='x'){
                aux+=1;
            }else aux+=0;

            numberOctal=numberOctal*10+aux;
            aux=0;

            if(permissions[6]=='r'){
                aux+=4;
            }
            else aux+=0;
            if(permissions[7]=='w'){
                aux+=2;
            }else aux+=0;
            if(permissions[8]=='x'){
                aux+=1;
            }else aux+=0;

            numberOctal=numberOctal*10+aux;
            aux=0;

            listingDirectorsPermissions(path, numberOctal);
            free(permissions);
            free(path);
            // printf("ksnvjsenvkjsenvknesv\n");
        }


        //parse listing

        if(strcmp(argv[1], "parse") == 0)
        {
            if(strstr(argv[2], "path=")){ //if we have parse path=

                int pathLength = strlen(argv[2]) - 5;
                char* path = (char*)malloc(pathLength*sizeof(char));

                strcpy(path, argv[2] + 5); //copy path syntax

                parsingFile(path);
                free(path);
            }
        }

        //section parse listing

        if(strcmp(argv[1], "extract")==0 && argc>=4){
            if(strstr(argv[2], "path=")){
                if(strstr(argv[3], "section=")){
                    if(strstr(argv[4], "line=")){ //if we have extract path= section= line=
                        int pathLength=strlen(argv[2])-5;
                        char *path=(char*)malloc(pathLength*sizeof(char));

                        int section=atoi(argv[3]+8); //extract section as int
                        int line=atoi(argv[4]+5); //extract line as int

                        strcpy(path, argv[2]+5); //copy path syntax

                        extractLine(path, section, line);

                        // printf("%s, %d, %d\n", path, section, line);
                        free(path);
                    }
                }
            }
        }

        //findall listing

         if(strcmp(argv[1], "findall") == 0 && strstr(argv[2], "path="))
	    {
             char* path = (char*)malloc(2048*sizeof(char)); //path needs more space

            strcpy(path, argv[2] + 5); //copy path syntax

            findAllFile(path, 1);

            free(path);
	    }
    }
    return 0;
}