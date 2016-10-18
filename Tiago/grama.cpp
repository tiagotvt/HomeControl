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
void mapping(Mat image, Mat mask, int sqr_row, int sqr_col, int blockSize, int sqr, vector<mapBlock> &blockMap)
{
  int posX = 0, posY = 0, row, col, temp=0, sqr_num=0;
  /* Passar o vetor por address*/
  /* Colocar no bloquinho (0,0) do BLOCK, no centro dele */
  posX = (sqr/2);
  posY = (sqr/2);

  /**********************/
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
            //printf("ENTROU %d %d %d  X%d  Y%d\n", image.at<Vec3b>(posY, posX)[0], image.at<Vec3b>(posY, posX)[1], image.at<Vec3b>(posY, posX)[2],posX,posY);
            circle(image, Point(posX,posY), 10, Scalar(0,0,255),-1,8);
            temp ++;
            
          }
        }
        sqr_num ++;
        posX += sqr;
      }
      posX = (sqr/2);
      posY += sqr;
    }
  }

  printf("TESTEEEEEEEE");
}

int main( int argc, char** argv )
{
  //Command to get the weather file
  system("curl wttr.in/'Sao Carlos' > rain.txt");
  range GRASS, good;

  //Gets the Range of healthy grass from a file.
  ifstream ipfile("data.txt");
  ipfile >> good.min[0]>>good.min[1]>>good.min[2];
  ipfile >> good.max[0]>>good.max[1]>>good.max[2];

  //GRASS.min = Scalar(22, 30, 70);
  GRASS.min = Scalar(12, 50, 30);
  GRASS.max = Scalar(80,255,200);
  //good.min = Scalar(28, 100, 100);
  //good.max = Scalar(80, 255, 255);
  ipfile.close();


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

  imwrite( "HSV.jpg", src );
  src.release();

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
    //imwrite( "sqr.jpg", final_sqr );
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
  vector<mapBlock> blockMap;
  vector<block> blockError;
  mapping(RGB, mask2, sqr_row, sqr_col, blockSize, sqr, blockMap);
  for(tmp=0; tmp < blockMap.size(); tmp++)
  {
    printf ("%d ", blockMap[tmp].value);
    if(blockMap[tmp].value)
    {
      printf("blocox %d     blocoy %d      posX %d     posY %d\n", blockMap[tmp].block_numX, blockMap[tmp].block_numY, blockMap[tmp].posX, blockMap[tmp].posY);
    }
  }

  makeGrid(RGB, blockSize);
  
  block blockTemp;
  bool notSameBlock = true;
  int tmp2;
  
  for(tmp=0; tmp < sprinklerVec.size(); tmp++)
  {
    circle(RGB, Point(sprinklerVec[tmp].posX,sprinklerVec[tmp].posY), 10, Scalar(0,255,255),-1,8);
    cout << sprinklerVec[tmp].posX<<" "<<sprinklerVec[tmp].posY<<" "<<sprinklerVec[tmp].range<<" "<<sprinklerVec[tmp].angle<<" "<<sprinklerVec[tmp].orientation<<"\n";
  }

//printf("%f %f %f\n",good.min[0],good.min[1],good.min[2]);
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
  imwrite( "sqr.jpg", final_sqr );
  imwrite( "MASK.jpg", mask );
  imwrite( "MASK2.jpg", mask2 );
  imwrite( "threshold.jpg", threshold );
  imwrite( "dst.jpg", dst );
  
  imwrite( "final.jpg", final );

  
  imwrite( "grid.jpg", final_sqr );

  //imshow( "source", src );
  //imshow( "threshold", threshold );
  //imshow( "dst", dst );
  printf("%d\n", sqr_col);
  printf("%d\n", sqr_row);
  isRaining();
  return 0;
}