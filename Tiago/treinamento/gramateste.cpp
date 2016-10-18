#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

//"g++ $(pkg-config --cflags opencv) grama.cpp $(pkg-config --libs opencv) -o grama"

using namespace cv;
using namespace std;
#define PI 3.14159265
struct range {
  Scalar min;
  Scalar max;
};

struct block {
  int x;
  int y;
};

struct mapBlock {
  int posX;
  int posY;
  bool value = false;
  int block_numX;
  int block_numY;
};

struct sprinkler {
  int posX;
  int posY;
  int range;
  int angle;
  int orientation;
};

vector<Point> userPoints;

/*  This function gets the MAX value of the new Range used to decide
*   if the grass is healthy or not.
*     
*   () -> Range
*   x  -> AVG value
*   +  -> distance  (AVG - Range.min) and (AVG - Range.max)
*
*   Algorithim used for the False Positive
*   -We need to "put the squares that are wrong inside the range", so
*    we just need to get the biggest value of them and use as Range.max
*
*   Algorithim used for the False Negative
*   -We need to try to "put some squares out of the range", so we need
*    to shrink our range to get a great amout of squares, not just the 
*    two closer to the ranges.
*
*   NORMAL CASE
*   (-----------+-----------x-----------+-----------)
* RgMin                    AVG                    RgMax
*
*   When AVG is closer to the Range.max
*   (--------------+----------------x-------+-------)
* RgMin                            AVG            RgMax
*
*   When AVG is closer to the Range.min
*   (-------+-------x---------------+---------------)
* RgMin            AVG                           RgMax
*
*   (--------------+--------------+---------------)
*               Dist.min       Dist.max
*
*                  (--------------)   NEW RANGE
*/
Scalar getBiggest (vector<block> falseBlocks, vector<mapBlock> squares, bool type, Mat image, range rangeValue)
{
  Scalar newMaxValue = image.at<Vec3b>(squares[0].posY, squares[0].posX);
  Scalar tempMaxValue, bigMaxValue = rangeValue.max, avgMaxValue = (0,0,0);
  int i, j, cont = 0;
  //if type == true, false positive; if type == false, false negative;
  if(type)
  {
    for(i=0; i < squares.size(); i++)
    {
      if(newMaxValue[0] < image.at<Vec3b>(squares[i].posY, squares[i].posX)[0])
        newMaxValue[0] = image.at<Vec3b>(squares[i].posY, squares[i].posX)[0];
      if(newMaxValue[1] < image.at<Vec3b>(squares[i].posY, squares[i].posX)[1])
        newMaxValue[1] = image.at<Vec3b>(squares[i].posY, squares[i].posX)[1];
      if(newMaxValue[2] < image.at<Vec3b>(squares[i].posY, squares[i].posX)[2])
        newMaxValue[2] = image.at<Vec3b>(squares[i].posY, squares[i].posX)[2];
    }
    //Gets the biggest BGR value of the marked squares
    //Change only the values that bigger than the initial range
    if (newMaxValue[0] < rangeValue.max[0])
      newMaxValue[0] = rangeValue.max[0]; 
    if (newMaxValue[1] < rangeValue.max[1])
      newMaxValue[1] = rangeValue.max[1]; 
    if (newMaxValue[2] < rangeValue.max[2])
      newMaxValue[2] = rangeValue.max[2];

    return newMaxValue;
  }

  for(i=0; i < falseBlocks.size(); i++)
  {
    for(j=0; j < squares.size(); j++)
    {
      if((falseBlocks[i].x == squares[j].block_numX) && (falseBlocks[i].y == squares[j].block_numY))
      {
        //Get the average color value of the Blocks that were considered FALSE NEGATIVE
        avgMaxValue[0] += image.at<Vec3b>(squares[j].posY, squares[j].posX)[0];
        avgMaxValue[1] += image.at<Vec3b>(squares[j].posY, squares[j].posX)[1];
        avgMaxValue[2] += image.at<Vec3b>(squares[j].posY, squares[j].posX)[2];
        cont++;
      }
    }
    tempMaxValue[0] = ceil(avgMaxValue[0]/cont);
    tempMaxValue[1] = ceil(avgMaxValue[1]/cont);
    tempMaxValue[2] = ceil(avgMaxValue[2]/cont);


    //Receives the smallest AVG
    if(bigMaxValue[0] > tempMaxValue[0])
      bigMaxValue[0] = tempMaxValue[0];
    if(bigMaxValue[1] > tempMaxValue[1])
      bigMaxValue[1] = tempMaxValue[1];
    if(bigMaxValue[2] > tempMaxValue[2])
      bigMaxValue[2] = tempMaxValue[2];

    cout << "\n " << cont << " MAX " << tempMaxValue << " \n";


    avgMaxValue = (0,0,0);
    cont = 0;
  }
  cout << " MENOR " << bigMaxValue << "\n";
  newMaxValue[0] = ceil(rangeValue.max[0] + (bigMaxValue[0] - rangeValue.max[0])/2);
  newMaxValue[1] = ceil(rangeValue.max[1] + (bigMaxValue[1] - rangeValue.max[1])/2);
  newMaxValue[2] = ceil(rangeValue.max[2] + (bigMaxValue[2] - rangeValue.max[2])/2);
  return newMaxValue;
}

/*  This function gets the MIN value of the new Range used to decide
*   if the grass is healthy or not.
*
*   Algorithim used for the False Positive
*   -We need to "put the squares that are wrong inside the range", so
*    we just need to get the smallest value of them and use as Range.min.
*/
Scalar getSmallest (vector<block> falseBlocks, vector<mapBlock> squares, bool type, Mat image, range rangeValue)
{
  //if type == true, false positive; if type == false, false negative;
  Scalar newMinValue = image.at<Vec3b>(squares[0].posY, squares[0].posX);
  Scalar tempMinValue, bigMinValue = rangeValue.min, avgMinValue = (0,0,0);
  int i, j;
  int cont = 0;
  //if type == true, false positive; if type == false, false negative;
  if(type)
  {
    for(i=0; i < squares.size(); i++)
    {
      if(newMinValue[0] > image.at<Vec3b>(squares[i].posY, squares[i].posX)[0])
        newMinValue[0] = image.at<Vec3b>(squares[i].posY, squares[i].posX)[0];
      if(newMinValue[1] > image.at<Vec3b>(squares[i].posY, squares[i].posX)[1])
        newMinValue[1] = image.at<Vec3b>(squares[i].posY, squares[i].posX)[1];
      if(newMinValue[2] > image.at<Vec3b>(squares[i].posY, squares[i].posX)[2])
        newMinValue[2] = image.at<Vec3b>(squares[i].posY, squares[i].posX)[2];
    }
    //Gets the biggest BGR value of the marked squares
    //Change only the values that bigger than the initial range
    if (newMinValue[0] > rangeValue.max[0])
      newMinValue[0] = rangeValue.max[0]; 
    if (newMinValue[1] > rangeValue.max[1])
      newMinValue[1] = rangeValue.max[1]; 
    if (newMinValue[2] > rangeValue.max[2])
      newMinValue[2] = rangeValue.max[2];

    return newMinValue;
  }

  for(i=0; i < falseBlocks.size(); i++)
  {
    for(j=0; j < squares.size(); j++)
    {
      if((falseBlocks[i].x == squares[j].block_numX) && (falseBlocks[i].y == squares[j].block_numY))
      {
        //Get the average color value of the Blocks that were considered FALSE NEGATIVE
        avgMinValue[0] += image.at<Vec3b>(squares[j].posY, squares[j].posX)[0];
        avgMinValue[1] += image.at<Vec3b>(squares[j].posY, squares[j].posX)[1];
        avgMinValue[2] += image.at<Vec3b>(squares[j].posY, squares[j].posX)[2];
        cont++;
      }
    }
    tempMinValue[0] = floor(avgMinValue[0]/cont);
    tempMinValue[1] = floor(avgMinValue[1]/cont);
    tempMinValue[2] = floor(avgMinValue[2]/cont);

    //Receives the biggest AVG
    if(bigMinValue[0] < tempMinValue[0])
      bigMinValue[0] = tempMinValue[0];
    if(bigMinValue[1] < tempMinValue[1])
      bigMinValue[1] = tempMinValue[1];
    if(bigMinValue[2] < tempMinValue[2])
      bigMinValue[2] = tempMinValue[2];

    cout << "\n " << cont << " MIN " << tempMinValue << " \n";
    avgMinValue = (0,0,0);
    cont = 0;
  }
  cout << " MAIOR " << bigMinValue << "\n";
  newMinValue[0] = floor(rangeValue.min[0] + ((bigMinValue[0] - rangeValue.min[0])/2));
  newMinValue[1] = floor(rangeValue.min[1] + ((bigMinValue[1] - rangeValue.min[1])/2));
  newMinValue[2] = floor(rangeValue.min[2] + ((bigMinValue[2] - rangeValue.min[2])/2));

  return newMinValue;
}
/*  This function tests if we had any False Positive blocks.
*   We compare the blocks clicked by the user and the blocks
*   considered unhealty grass by our program.
*
*   -The vector blockMap already has all the squares considered unhealthy
*/
bool testFalsePositive(vector<block> programBlocks, vector<block> userBlocks, vector<block> &falseBlocks)
{
  int i=0;
  int j=0;
  bool flag = true;
  while(i < programBlocks.size())
  {
    for(j=0; j< userBlocks.size();j++)
    {
      if((programBlocks[i].x == userBlocks[j].x) && (programBlocks[i].y == userBlocks[j].y))
        flag = false;
    }
    if(flag)
    {
      falseBlocks.push_back(programBlocks[i]);
    }
    i++;
    flag = true;
  }

  //return true if there are false positives, and the vector receives the values
  //return false if there are NOT false positives
  return (falseBlocks.size()>0);
}
/*  This function tests if we had any False Negative blocks.
*   We compare the blocks clicked by the user and the blocks
*   considered unhealty grass by our program.
*   
*   -The vector blockMaskMap already has all the squares considered unhealthy
*/
bool testFalseNegative(vector<block> programBlocks, vector<block> userBlocks, vector<block> &falseBlocks)
{
  int i=0;
  int j=0;
  bool flag = true;
  while(i < userBlocks.size())
  {
    for(j=0; j< programBlocks.size();j++)
    {
      if((userBlocks[i].x == programBlocks[j].x) && (userBlocks[i].y == programBlocks[j].y))
        flag = false;
    }
    if(flag)
    {
      falseBlocks.push_back(userBlocks[i]);
    }
    i++;
    flag = true;
  }

  //return true if there are false negatives, and the vector receives the values
  //return false if there are NOT false negatives
  return (falseBlocks.size()>0);
}

/*  This function gets the % of raining from the weather file.
*   It saves 4 values.
*   [CurrentWeather  Morning  Noon  Evening  Night]
*
*/
void isRaining()
{
  ifstream rainfile("rain.txt");
  ofstream rain2file("rainFinal.txt");
  string rainStr;
  int i=0;
  while ( i < 4 )
  {
    rainfile >> rainStr;
    size_t found = rainStr.find('%');
    if(found != -1)
    {
      cout << rainStr << " ";
      rain2file << rainStr.substr(0, found) << " ";
      i++;
    }
  }
  rain2file.close();
  rainfile.close();
}

/*  This function returns the distance between a block and a sprinkler.
*
*/
float dist (sprinkler sp, int x, int y, int blockSize)
{
  float posx=0, posy=0, difX, difY;
  if(x == 5)
    posx += (x*blockSize)+(blockSize/10);
  else posx += (x*blockSize)+(blockSize/2);
  posy += (y*blockSize)+(blockSize/2);
  difX = abs(posx-sp.posX);
  difY = abs(posy-sp.posY);

  return (sqrt(pow(difY,2)+pow(difX,2)));
}

/*  This function returns the angle that a certain sprinkler 
*   needs to have to water a certain block.
*
*/
int calc_angle(sprinkler sp, int x, int y, int blockSize)
{
  int posx=0, posy=0, difX, difY;
  posx += (x*blockSize)+(blockSize/2);
  posy += (y*blockSize)+(blockSize/2);
  difX = posx-sp.posX;
  difY = posy-sp.posY;
  switch(sp.orientation)
  {
    case 0:
      if(difX < 0)
      {
        return (atan(abs(difX/difY))*(180/PI))+90;
      }
      else return 90-(atan(abs(difX/difY))*(180/PI));
    
    case 1:
      if(difY > 0)
      {
        return (atan(abs(difY/difX))*(180/PI))+90;
      }
      else return 90-(atan(abs(difX/difY))*(180/PI));

    case 2:
      if(difY < 0)
      {
        return (atan(abs(difY/difX))*(180/PI))+90;
      }
      else return 90-(atan(abs(difX/difY))*(180/PI));

    case 3:
      if(difX > 0)
      {
        return (atan(abs(difX/difY))*(180/PI))+90;
      }
      else return 90-(atan(abs(difX/difY))*(180/PI));
  } 
}

/*  This function gets the event of LEFT MOUSE CLICK and
*   put the coordinates in a GLOBAL VECTOR.
*   (couldn't make it without the global variable)
*
*/
void CallBackFunc (int event, int x, int y, int flags, void* userdata)
{
  if(event == EVENT_LBUTTONDOWN)
  {
    cout << "Click in (" << x << ", " << y << ")" << endl;
    userPoints.push_back(Point(x,y)); 
  }
}

/*  This function simply draws a red grid in the image
*   depending the size of the blocks.
*
*/
void makeGrid(Mat image, int step)
{
  int width = image.size().width;
  int height = image.size().height;

  for (int i = 0; i<height; i += step)
      line(image, Point(0, i), Point(width, i), Scalar(0, 0, 255),2);

  for (int i = 0; i<width; i += step)
      line(image, Point(i, 0), Point(i, height), Scalar(0, 0, 255),2);
}

/*  This function "maps" the squares that are considered unhealty grass
*   and put all the [(x,y) square position, (x,y) block position] in a
*   vector of structs. 
*
*/
void mapping(Mat image, Mat mask, int sqr_row, int sqr_col, int blockSize, int sqr, vector<mapBlock> &blockMap, vector<mapBlock> &blockMaskMap)
{
  int posX = 0, posY = 0, row, col, temp=0, temp2=0, sqr_num=0;
  /* Passar o vetor por address*/
  /* Colocar no bloquinho (0,0) do BLOCK, no centro dele */
  posX = (sqr/2);
  posY = (sqr/2);

  while(sqr_num < sqr_row*sqr_col)
  {
    for (row=0; row < sqr_row; row++)
    {
      for (col=0; col < sqr_col; col++)
      {
        if(image.at<Vec3b>(posY, posX) != Vec3b(0,0,0))
        {
          if(mask.at<Vec3b>(posY, posX) == Vec3b(0,0,0))
          {
            blockMap.push_back(mapBlock());
            blockMap[temp].posX = posX;
            blockMap[temp].posY = posY;
            blockMap[temp].value = true;
            blockMap[temp].block_numX = (posX/(blockSize));
            blockMap[temp].block_numY = (posY/(blockSize));
            circle(image, Point(posX,posY), 10, Scalar(0,0,255),-1,8);
            temp ++;
            
          }
          else 
          {
            blockMaskMap.push_back(mapBlock());
            blockMaskMap[temp2].posX = posX;
            blockMaskMap[temp2].posY = posY;
            blockMaskMap[temp2].value = true;
            blockMaskMap[temp2].block_numX = (posX/(blockSize));
            blockMaskMap[temp2].block_numY = (posY/(blockSize));
            temp2 ++;
          }
        }
        sqr_num ++;
        posX += sqr;
      }
      posX = (sqr/2);
      posY += sqr;
    }
  }
}

int main( int argc, char** argv )
{
  //system("curl wttr.in/'Sao Carlos' > rain.txt");
  range GRASS, good;

  //Gets the Range of healthy grass from a file.
  ifstream ipfile("data.txt");
  ipfile >> good.min[0]>>good.min[1]>>good.min[2];
  ipfile >> good.max[0]>>good.max[1]>>good.max[2];
  ipfile.close();
  //GRASS.min = Scalar(22, 30, 70);
  //good.min = Scalar(28, 100, 100);
  //good.max = Scalar(80, 255, 255);

  GRASS.min = Scalar(12, 50, 30);
  GRASS.max = Scalar(80,255,200);

  int tmp;
  vector<sprinkler> sprinklerVec;
  ifstream spfile("sprinkler.txt");

  //Gets the sprinkler data from a file.
  for(tmp = 0; tmp < 6; tmp++)
  {
    sprinklerVec.push_back(sprinkler());
    spfile >> sprinklerVec[tmp].posX>>sprinklerVec[tmp].posY>>sprinklerVec[tmp].range>>sprinklerVec[tmp].angle>>sprinklerVec[tmp].orientation;
  }
  spfile.close();

  Mat edge,dst,threshold, final;
  Mat HSV, RGB;
  int dilation_size =4;
  int sqr = 100;
  int blockSize = sqr*5;
  Mat src = imread( argv[1],1);

//If the image is not a multiple of "sqr", we round it up
  int sqr_row = src.rows/sqr;  
  if (src.rows%sqr > 0) sqr_row = (src.rows/sqr)+1;

  int sqr_col = src.cols/sqr;
  if (src.cols%sqr > 0) sqr_col = (src.cols/sqr)+1;


  int MAX_BLACK_PIXEL = (sqr*sqr)*0.60;               //40% of the squared pixels can be black.

  blur( src, src, Size(2,2) );
  cvtColor(src,HSV,CV_BGR2HSV);                       //Transform a BGR image to a HSV. 
  inRange(HSV,GRASS.min,GRASS.max,threshold);         //Gets a threshold image from the initial image, based in the ranges
  Mat element = getStructuringElement( MORPH_RECT,Size( 2*dilation_size + 1, 2*dilation_size+1 ), Point( dilation_size, dilation_size ) );
  morphologyEx( threshold, dst, MORPH_CLOSE, element );
  

  int minBound=dst.rows;
  int maxBound=0;
  tmp=0;

  for(int i = 0; i < dst.cols; i++){  
     for(int j = 0; j < dst.rows; j++){
        if(dst.at<uchar>(j, i)==0){
        tmp=j;
        break;
        }
     }
    if(tmp<minBound) minBound=tmp;
    if(tmp>maxBound) maxBound=tmp;
  }

    // Copy all pixels from the original image that are not black in the threshold image.
    src.copyTo(final, threshold);

/* Starts to calculate the average pixel color by area
****************************************************************************************************/
  Vec3b cont;
  Mat final_sqr = Mat::zeros(sqr_row*sqr,sqr_col*sqr, CV_8UC3);
  int x_cont, y_cont, sqr_num=0,sqr_num_x=0,sqr_num_y=0, avg_temp=0, non_black=0;
  int blue_temp = 0;
  int green_temp = 0;
  int red_temp = 0;

  printf("TESTEEE\n %d %d %d",sqr_col*sqr_row, sqr_row, sqr_col);
  while(sqr_num < sqr_row*sqr_col)
  {
    printf("ALOOO");
     for(y_cont = (sqr*sqr_num_y); y_cont < (sqr*(sqr_num_y+1)); y_cont++) 
     {
      for(x_cont = (sqr*sqr_num_x); x_cont < (sqr*(sqr_num_x+1)); x_cont++) 
      {
          if ( final.at<Vec3b>(y_cont, x_cont) != Vec3b(0,0,0) && final.at<Vec3b>(y_cont, x_cont) != Vec3b(255,255,255) ) 
          {
            blue_temp += final.at<Vec3b>(y_cont, x_cont)[0];
            green_temp += final.at<Vec3b>(y_cont, x_cont)[1];
            red_temp += final.at<Vec3b>(y_cont, x_cont)[2];
            avg_temp ++;
            non_black ++;
          }
      }
    }
    printf("%d  ",sqr_num);
    printf("%d  ",sqr_num_y);
    printf("%d  \n",sqr_num_x);
    printf("TESTEEE2\n");
    if (non_black <= MAX_BLACK_PIXEL || non_black==0)
    {
      blue_temp = 0;
      green_temp = 0;
      red_temp = 0;
    }
    if(avg_temp==0) avg_temp=1;
    rectangle(final_sqr, Point(0+(sqr*sqr_num_x),(0+sqr*sqr_num_y)),Point((sqr*(sqr_num_x+1))-1,(sqr*(sqr_num_y+1))-1),
              Scalar(blue_temp/avg_temp,green_temp/avg_temp,red_temp/avg_temp),-1,8);
    blue_temp=0;
    green_temp=0;
    red_temp=0;
    avg_temp=0;
    non_black=0;

    sqr_num++;
    sqr_num_x = sqr_num%sqr_col;
    if(sqr_num_x == 0)
    {
      sqr_num_y++;
    }
    printf("%d  ",sqr_num);
    printf("%d  ",sqr_num_y);
    printf("%d  \n",sqr_num_x);
    printf("TESTEEE3\n");
  }
/* Ends to calculate the average pixel color by area
****************************************************************************************************/
/*******Testing*******/
  imwrite( "sqr.jpg", final_sqr );
  RGB = imread( "sqr.jpg");
  Mat mask,mask2;
  inRange(RGB,good.min,good.max,mask);

  //Copy all pixels from the squared image, that are not black in the mask image,to the new mask.
  final_sqr.copyTo(mask2, mask);
  vector<mapBlock> blockMap, blockMaskMap;
  vector<block> blockError;
  mapping(RGB, mask2, sqr_row, sqr_col, blockSize, sqr, blockMap, blockMaskMap);
  for(tmp=0; tmp < blockMap.size(); tmp++)
  {
    printf ("%d ", blockMap[tmp].value);
    if(blockMap[tmp].value)
    {
      printf("blocox %d     blocoy %d      posX %d     posY %d\n", blockMap[tmp].block_numX, blockMap[tmp].block_numY, blockMap[tmp].posX, blockMap[tmp].posY);
    }
  }

  makeGrid(RGB, blockSize);
  namedWindow ("Display Grid", WINDOW_FREERATIO);                   //Creates the window showed to the user
  imshow("Display Grid", RGB);
  setMouseCallback("Display Grid", CallBackFunc, 0);
  waitKey(0);                                                       //Waits a key to be pressed
  block blockTemp;
  bool notSameBlock = true;
  int tmp2;

  for(tmp=0; tmp < userPoints.size(); tmp++)
  {
    blockTemp.x = (userPoints[tmp].x/blockSize);
    blockTemp.y = (userPoints[tmp].y/blockSize); 

    for(tmp2=0; tmp2 < blockError.size(); tmp2++)
    {
      if((blockTemp.x == blockError[tmp2].x) && (blockTemp.y == blockError[tmp2].y))
        notSameBlock = false;
    }
    if(notSameBlock)
      blockError.push_back(blockTemp);

    notSameBlock = true;
  }

  for(tmp=0; tmp < blockError.size(); tmp++)
  {
    cout << blockError[tmp].x << " " << blockError[tmp].y <<"\n";
  }

  
  for(tmp=0; tmp < sprinklerVec.size(); tmp++)
  {
    circle(RGB, Point(sprinklerVec[tmp].posX,sprinklerVec[tmp].posY), 10, Scalar(0,255,255),-1,8);
    cout << sprinklerVec[tmp].posX<<" "<<sprinklerVec[tmp].posY<<" "<<sprinklerVec[tmp].range<<" "<<sprinklerVec[tmp].angle<<" "<<sprinklerVec[tmp].orientation<<"\n";
  }

  ofstream ipfile2;
  ipfile2.open("data.txt");
  ipfile2 << good.min[0]<< " " << good.min[1] << " " << good.min[2]<< "\n";
  ipfile2 << good.max[0]<< " " << good.max[1] << " " << good.max[2];
  ipfile2.close();
/********END TESTING************/

/********CHOOSE SPRINKLER*******/
vector<block> blockSprinkler;
  ofstream ipfile3;
  ipfile3.open("water.txt");
  

for(tmp=0; tmp < blockMap.size(); tmp++)
{
  blockTemp.x = (blockMap[tmp].block_numX);
  blockTemp.y = (blockMap[tmp].block_numY); 

  for(tmp2=0; tmp2 < blockSprinkler.size(); tmp2++)
  {
    if((blockTemp.x == blockSprinkler[tmp2].x) && (blockTemp.y == blockSprinkler[tmp2].y))
      notSameBlock = false;
  }
  if(notSameBlock)
    blockSprinkler.push_back(blockTemp);

  notSameBlock = true;
}

  for(tmp=0; tmp < blockSprinkler.size(); tmp++)
  {
    cout << blockSprinkler[tmp].x << " " << blockSprinkler[tmp].y <<"\n";
  }

  float hipo = 0, new_hipo = 0;
  int sp_num;
  for(tmp=0; tmp < blockSprinkler.size(); tmp++)
  {
    sp_num = 0;
    printf("BLOCK %d  %d\n",blockSprinkler[tmp].x, blockSprinkler[tmp].y );
    for(tmp2=0; tmp2 < sprinklerVec.size(); tmp2++)
    {
      new_hipo = dist (sprinklerVec[tmp2], blockSprinkler[tmp].x, blockSprinkler[tmp].y, blockSize);
      printf("%f \n", new_hipo);
      if(new_hipo < hipo)
      {
        printf("%d -> %d \n",sp_num, tmp2);
        sp_num = tmp2;
        hipo = new_hipo;
      }
    }
    hipo = 4000.0;
    if(blockSprinkler[tmp].x == 5)
    {
      line(RGB, Point((blockSprinkler[tmp].x*blockSize)+(sqr/2), (blockSprinkler[tmp].y*blockSize)+(blockSize/2)), Point(sprinklerVec[sp_num].posX, sprinklerVec[sp_num].posY), Scalar(0, 255, 255),2);
      cout <<"irraaaaaaa";
    }
    else line(RGB, Point((blockSprinkler[tmp].x*blockSize)+(blockSize/2), (blockSprinkler[tmp].y*blockSize)+(blockSize/2)), Point(sprinklerVec[sp_num].posX, sprinklerVec[sp_num].posY), Scalar(0, 255, 255),2);
    ipfile3 << sp_num << " " << calc_angle(sprinklerVec[sp_num], blockSprinkler[tmp].x, blockSprinkler[tmp].y, blockSize) << "\n";
  }

  ipfile3.close();
/******END CHOOSING SPRINKLER***/

/*************Test User's points and programs points**********/
vector<block> blockFalseP, blockFalseN;
bool flag = true;

if(testFalsePositive(blockSprinkler, blockError, blockFalseP))
{
  for(tmp=0; tmp < blockFalseP.size();tmp++)
  {
    cout << "Falso P "<< blockFalseP[tmp].x << " " << blockFalseP[tmp].y << "\n";
  }
}
if(testFalseNegative(blockSprinkler, blockError, blockFalseN))
{
  for(tmp=0; tmp < blockFalseN.size();tmp++)
  {
    cout << "Falso N "<< blockFalseN[tmp].x << " " << blockFalseN[tmp].y << "\n";
  }
}

//Leave just the squares that are considered False Positives
for(tmp=0; tmp < blockMap.size(); tmp++)
{

  for(tmp2=0; tmp2 < blockFalseP.size(); tmp2++)
  {
    if((blockMap[tmp].block_numX == blockFalseP[tmp2].x) && (blockMap[tmp].block_numY == blockFalseP[tmp2].y))
    {
      flag = false;
    }
  }
  if(flag)
  {
    cout << "Deletado " << blockMap[tmp].block_numX << " and " << blockMap[tmp].block_numY << "\n";
    cout << tmp;
    cout << blockMap.size();
    if(blockMap.size() == 1)
    {
      blockMap.push_back(mapBlock());
      blockMap.erase(blockMap.begin() + tmp);
      break;
    }
    blockMap.erase(blockMap.begin() + tmp);
    tmp --;

  }
  flag = true;
}

//Leave just the squares that are considered False Negatives
for(tmp=0; tmp < blockMaskMap.size(); tmp++)
{

  for(tmp2=0; tmp2 < blockFalseN.size(); tmp2++)
  {
    if((blockMaskMap[tmp].block_numX == blockFalseN[tmp2].x) && (blockMaskMap[tmp].block_numY == blockFalseN[tmp2].y))
    {
      flag = false;
    }
  }
  if(flag)
  {
    cout << "Deletado " << blockMaskMap[tmp].block_numX << " and " << blockMaskMap[tmp].block_numY << "\n";
    cout << tmp << "  ";
    cout << blockMaskMap.size()<< "  ";
    if(blockMaskMap.size() == 1)
    {
      blockMaskMap.push_back(mapBlock());
      blockMaskMap.erase(blockMaskMap.begin() + tmp);
      break;
    }
    blockMaskMap.erase(blockMaskMap.begin() + tmp);
    tmp --;

  }
  flag = true;
}


cout << " \n OS FALSE NEGATIVOS \n";
for(tmp=0;tmp < blockMaskMap.size();tmp++)
{
  cout << blockMaskMap[tmp].block_numX << "  " << blockMaskMap[tmp].block_numY << "\n" ;
}

tmp = 0;
range newRange;


newRange.max = (0,0,0);

if(blockFalseN.size()>0)
{
  newRange.max = getBiggest (blockFalseN, blockMaskMap, false, final_sqr, good);
  newRange.min = getSmallest (blockFalseN, blockMaskMap, false, final_sqr, good);
  cout << "\n TEVE FALSO - \n";
}
if(blockFalseP.size()>0)
{
  newRange.max = getBiggest (blockFalseP, blockMap, true, final_sqr, good);
  newRange.min = getSmallest (blockFalseP, blockMap, true, final_sqr, good);
  cout << "\n TEVE FALSO + \n";
}

if(blockFalseP.size()>0 || blockFalseN.size()>0)
  cout << "MAX " << newRange.max << "  MIN " << newRange.min << "\n";
/*************End testing and modifying ranges **************/
  imwrite( "sqr.jpg", final_sqr );
  imwrite( "rgb.jpg", RGB );
  imwrite( "MASK.jpg", mask );
  imwrite( "MASK2.jpg", mask2 );
  imwrite( "threshold.jpg", threshold );
  imwrite( "dst.jpg", dst );
  imwrite( "HSV.jpg", src );
  imwrite( "final.jpg", final );
  imwrite( "grid.jpg", final_sqr );

  printf("%d\n", sqr_col);
  printf("%d\n", sqr_row);
  //isRaining();
  return 0;
}