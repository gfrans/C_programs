/* TODO:
   * sort out what should be global/local state
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// default maze dimensions
#define HEIGHT 50
#define WIDTH  60

// for direction, 0 is toward 0 on x axis
// 1 is toward 0 on y axis
// 2 is toward inifinity on x axis
// 3 is toward inifinity on y axis
#define LEFT    0
#define UP      1
#define RIGHT   2
#define DOWN    3

// for the maze, 0 is path and 1 is wall
// 2 is start and 3 is end
#define PATH_VAL     1 << 0 // 1
#define WALL_VAL     1 << 1 // 2
#define START_VAL    1 << 2 // 4
#define END_VAL      1 << 3 // 8

// characters for printing the maze
#define PATH_CHAR   '0'
#define WALL_CHAR   '.'
#define START_CHAR  '4'
#define END_CHAR    '8'

// determine if maze should be filled with false paths
#define FILL_MAZE   1

// minimum fill percentage when FILL_MAZE is set
#define FILL_PERCENT 20

// derermine max length of added paths
#define FALSE_LEN   5

// maximum number of cycles to try constructing maze before aborting
#define CYCLE_LIMIT 0xfffff

typedef struct _Coord
{
    uint8_t x;
    uint8_t y;
} Coord;

// undo mistakes flag (1: undo, 0: don't)
uint8_t fillMaze = FILL_MAZE;

// flase path length
uint8_t falseLen = FALSE_LEN;

// cycle limit
uint32_t cycleCount = 0;

// coords is an array of two coords; one for start, one for end
// returns 0 on success (placed start/end), 1 on failure
// separation is the minimum distance around periphery between start/end
uint8_t initMaze(Coord *coords,
                 uint8_t width,
                 uint8_t height,
                 uint8_t separation)
{
    uint8_t i = 0, j = 0;
    uint16_t offsetArray[2] = {0};
    uint16_t perimeter = (2 * width + 2 * height);

    // add check for overlapping start/end; i-- and continue
    for (i = 0; i < 2; i++)
    {
        // calculate the offset around perimeter of maze
        offsetArray[i] =  rand() % perimeter;

        printf("i %d, offset %#x\n", i, offsetArray[i]);

        // if offset falls in first row
        if (offsetArray[i] < width)
        {
            coords[i].y = 0;
            coords[i].x = offsetArray[i];
        }
        // if offset falls on right side
        else if (offsetArray[i] < (width + height))
        {
        }
        // if offset falls on bottom edge
        else if (offsetArray[i] < (2 * width + height))
        {
            coords[i].y = height - 1;
            coords[i].x = width - (offsetArray[i] - (width + height) + 1);
        }
        else // offset falls on left edge
        {
            coords[i].y = height - (offsetArray[i] - (2 * width + height) + 1);
            coords[i].x = 0;
        }

        // if this sqaure was already used as an entry/exit point
        if ((i > 0) &&
            ((abs(coords[i - 1].x - coords[i].x) +
              abs(coords[i - 1].y - coords[i].y)) < separation))
        {
            offsetArray[i] = rand() % ((2 * height) + (2 * width));
            i--;
        }
    }

    return 0;
}

uint8_t isValid(uint8_t xPos,
                uint8_t yPos,
                uint8_t **maze,
                uint8_t width,
                uint8_t height)
{
    uint8_t neighbors = 0;

    // check if square is in-bounds
    if ((xPos >= width) || (yPos >= height))
    {
        return 0;
    }

    // you can't tread backwards
    if (maze[yPos][xPos] == PATH_VAL)
    {
        return 0;
    }

    // check for neighbor to left
    if ((xPos > 0) && (maze[yPos][xPos - 1] == PATH_VAL))
    {
        neighbors++;
    }
    // check for neighbor up
    if ((yPos > 0) && (maze[yPos - 1][xPos] == PATH_VAL))
    {
        neighbors++;
    }
    // check for neighbor to right
    if ((xPos < (width - 1)) && (maze[yPos][xPos + 1] == PATH_VAL))
    {
        neighbors++;
    }
    // check for neighbor below
    if ((yPos < (height - 1)) && (maze[yPos + 1][xPos] == PATH_VAL))
    {
        neighbors++;
    }

    return (neighbors < 2);
}

// returns 0 on success, 1 on failure
// falsePaths -- determines if current pos should revert to wall on failed path
// maxLength -- max length of path segment; set to WIDTH * HEIGHT to ignore
uint8_t genPath(Coord cur,
                Coord exit,
                uint8_t **maze,
                uint8_t width,
                uint8_t height,
                uint8_t falsePaths,
                uint16_t maxLength)
{
    uint8_t explored = 0, next = 0, foundExit = 0;
    Coord nextCoord = {0};

    // if we reached the maxiumum path length, success
    if (maxLength == 0)
    {
        return 0;
    }

    // only continue if this position is valid
    if (isValid(cur.x, cur.y, maze, width, height))
    {
        // create a path segment at current position
        maze[cur.y][cur.x] = PATH_VAL;

        // base case, found exit square
        if (((cur.y == exit.y) && (((cur.x - exit.x) == 1) || ((exit.x - cur.x) == 1))) ||
            ((cur.x == exit.x) && (((cur.y - exit.y) == 1) || ((exit.y - cur.y) == 1))))
        {
            maze[exit.y][exit.x] = PATH_VAL;
            return 0;
        }

        // randomly pick a neighbor to explore until exhausted or path complete
        while (explored < 15)
        {
            // start at current position
            nextCoord.x = cur.x;
            nextCoord.y = cur.y;

            // generate the next direction to check
            do
            {
                next = rand() % 4;
            }
            while (explored & (1 << next));

            explored |= (1 << next);

            // direction is left
            if (next == 0)
            {
                // set next Coord
                nextCoord.x--;
            }
            // direction is up
            else if (next == 1)
            {
                nextCoord.y--;
            }
            // direction is right
            else if (next == 2)
            {
                nextCoord.x++;
            }
            // direction is down
            else // next == 3
            {
                nextCoord.y++;
            }

            // make recursive call
            if (genPath(nextCoord,
                        exit,
                        maze,
                        width,
                        height,
                        falsePaths,
                        maxLength - 1) == 0)
            {
                // if we just want the one path, we're done
                if (!fillMaze)
                {
                    return 0;
                }
                // otherwise, start creating false paths
                else
                {
                    maxLength = (maxLength > falseLen) ? falseLen : maxLength;
                    foundExit = 1;
                }
            }

            // check if we should abort this seed
            if (!foundExit)
            {
                cycleCount++;
            }
            if (cycleCount >= CYCLE_LIMIT)
            {
                return 1;
            }
        }

        // disabling reversion stops the infinite walking problem
        // and helps fill the maze faster
        //
        // revert this position back to a wall (if it was turned into a path)
        if ((!falsePaths) && (!foundExit) && maze[cur.y][cur.x] == PATH_VAL)
        {
            maze[cur.y][cur.x] = WALL_VAL;
        }
    }

    return (!foundExit); // return failure
}


void printMaze(uint8_t **maze, uint8_t width, uint8_t height, Coord *coords)
{
    int h = 0, w = 0;

    for (h = 0; h < height; h++)
    {
        for (w = 0; w < width; w++)
        {
            if ((w == coords[0].x) && (h == coords[0].y))
            {
                printf("%c", START_CHAR);
            }
            else if ((w == coords[1].x) && (h == coords[1].y))
            {
                printf("%c", END_CHAR);
            }
            else if (maze[h][w] == PATH_VAL)
            {
                printf("%c", PATH_CHAR);
            }
            else if (maze[h][w] == WALL_VAL)
            {
                printf("%c", WALL_CHAR);
            }
        }

        printf("\n");
    }
}

uint8_t percentFull(uint8_t **maze, uint8_t width, uint8_t height)
{
    uint8_t h = 0, w = 0;
    uint16_t count = 0, max = (width * height);

    for (h = 0; h < height; h++)
    {
        for (w = 0; w < width; w++)
        {
            if (maze[h][w] == PATH_VAL)
            {
                count++;
            }
        }
    }

    return (uint8_t)(((1.0 * count) / max) * 100);
}


void main(int argc, char **argv)
{
    int i = 0;
    Coord endpoints[2] = {0};
    time_t seed;
    uint8_t height = HEIGHT, width = WIDTH;
    uint8_t done = 0;
    uint8_t sepDist = 0;

    // get maze dimensions
    if (argc >= 3)
    {
        height = strtoul(argv[1], NULL, 10);
        width = strtoul(argv[2], NULL, 10);
    }
    sepDist = width >> 1;

    // get fill flag
    if (argc >= 4)
    {
        fillMaze = strtoul(argv[3], NULL, 10);
    }

    // get false path length
    if (argc >= 5)
    {
        falseLen = strtoul(argv[4], NULL, 10);
    }

    // get start/end separation value
    if (argc == 6)
    {
        sepDist = strtoul(argv[5], NULL, 10);
    }

    // create a maze
    uint8_t mazeArr[height][width];

    // create the double-pointer version of maze array
    uint8_t *maze[height];
    for (i = 0; i < height; i++)
    {
        maze[i] = &(mazeArr[i][0]);
    }

    // initial seed
    seed = time(NULL);
    printf("seed %#x\n", seed);
    srand(seed);

    // set start/end points
    initMaze(endpoints, width, height, sepDist);

    // set the maze to all walls
    memset(mazeArr, WALL_VAL, height * width * sizeof(uint8_t));

    // print the starting state
    printf("Start: %#x, %#x; End: %#x, %#x\n", endpoints[0].x, endpoints[0].y, endpoints[1].x, endpoints[1].y);
    printMaze(maze, width, height, endpoints);
    printf("\n\n");

    while (!done)
    {
        // set the maze to all walls
        memset(mazeArr, WALL_VAL, height * width * sizeof(uint8_t));

        // clear the cycle count
        cycleCount = 0;

        // generate the maze
        if (genPath(endpoints[0], endpoints[1], maze, width, height, 0, width * height) == 0)
        {
            // ensure the maze is full enough
            if (fillMaze && (percentFull(maze, width, height) < FILL_PERCENT))
            {
                printf("Maze not fleshed-out enough... trying again\n");
            }
            else
            {
                done = 1;
                printMaze(maze, width, height, endpoints);
            }
        }
        else
        {
            printf("Aborted a maze... trying again\n");
        }
    }
}
