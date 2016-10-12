#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

//"g++ $(pkg-config --cflags opencv) grama.cpp $(pkg-config --libs opencv) -o grama"

using namespace cv;
using namespace std;
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
vector<Point> userPoints;

void CallBackFunc (int event, int x, int y, int flags, void* userdata)
{

  if(event == EVENT_LBUTTONDOWN)
  {
    cout << "Click in (" << x << ", " << y << ")" << endl;
    userPoints.push_back(Point(x,y)); 
  }
}

void makeGrid(Mat image, int step)
{
  int width = image.size().width;
  int height = image.size().height;

  for (int i = 0; i<height; i += step)
      line(image, Point(0, i), Point(width, i), Scalar(0, 0, 255),2);

  for (int i = 0; i<width; i += step)
      line(image, Point(i, 0), Point(i, height), Scalar(0, 0, 255),2);
}

bool testBlock(Mat image, Mat mask, int blockX, int blockY, int blockSize, int sqr, range grass, range outlier)
{
  int posX = 0, posY = 0;
  /* Passar o vetor por address*/
  /* Colocar no bloquinho (0,0) do BLOCK, no centro dele */
  posX += (blockX*blockSize*sqr)+(sqr/2);
  posY += (blockY*blockSize*sqr)+(sqr/2);


  int col,row;

  for (row=0; row < blockSize; row++)
  {
    for (col=0; col < blockSize; col++)
    {
      posX = 0;
    }
  }
}

//void mapping(Mat image, Mat mask, int blockX, int blockY, int blockSize, int sqr, map blockMap[])
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
  /*********************/
  /*
  for (row=0; row < blockSize; row++)
  {
    for (col=0; col < blockSize; col++)
    {
      if(image.at<Vec3b>(posY, posX) != Vec3b(0,0,0))
      {
        blockMap[temp].posX = posX;
        blockMap[temp].posY = posY;
      }
    posX += (col+1)*sqr;
    if(posX > image.cols) break;
    }

    posX = (blockX*blockSize*sqr)+(sqr/2);    //volta para o começo das colunas
    posY += (row+1)*sqr;
    if(posY > image.rows) break;
    
  }
  */
  printf("TESTEEEEEEEE");
}

int main( int argc, char** argv )
{
  range GRASS, good;
  ifstream ipfile("data.txt");
  ipfile >> good.min[0]>>good.min[1]>>good.min[2];
  ipfile >> good.max[0]>>good.max[1]>>good.max[2];

  //GRASS.min = Scalar(22, 30, 70);
  GRASS.min = Scalar(12, 50, 30);
  GRASS.max = Scalar(80,255,200);
  //good.min = Scalar(28, 100, 100);
  //good.max = Scalar(80, 255, 255);
  ipfile.close();

  

  Mat edge,dst,threshold, final;
  Mat HSV, RGB;
  int dilation_size =4;
  int sqr = 100;
  int blockSize = sqr*5;
  Mat src = imread( argv[1],1);

  int sqr_row = src.rows/sqr;  
  if (src.rows%sqr > 0) sqr_row = (src.rows/sqr)+1;

  int sqr_col = src.cols/sqr;
  if (src.cols%sqr > 0) sqr_col = (src.cols/sqr)+1;


  int MAX_BLACK_PIXEL = (sqr*sqr)*0.60;  //No máximo 30% dos pixels por quadrado pode ser preto

  blur( src, src, Size(2,2) );
  cvtColor(src,HSV,CV_BGR2HSV);
  inRange(HSV,GRASS.min,GRASS.max,threshold);
  Mat element = getStructuringElement( MORPH_RECT,Size( 2*dilation_size + 1, 2*dilation_size+1 ), Point( dilation_size, dilation_size ) );
  morphologyEx( threshold, dst, MORPH_CLOSE, element );
  

  int minBound=dst.rows;
  int maxBound=0;
  int tmp=0;

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

    // Copia todos os pixels da imagem original que não são pretos no threshold
    src.copyTo(final, threshold);

  //line(dst, Point(0,minBound), Point(dst.cols,minBound),  Scalar(0), 1,8, 0); //minmum boudary
  //line(dst, Point(0,maxBound), Point(dst.cols,maxBound),  Scalar(255), 1,8, 0); // maximum boundary
  //line(src, Point(0,minBound), Point(dst.cols,minBound),  Scalar(0,0,255), 1,8, 0); //minmum boudary
  //line(src, Point(0,maxBound), Point(dst.cols,maxBound),  Scalar(0,0,255), 1,8, 0); // maximum boundary

    // Fazer o Average da cor dos pixels por area


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
          if ( final.at<Vec3b>(y_cont, x_cont) != Vec3b(0,0,0) ) 
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
/*******Testing*******/
  imwrite( "sqr.jpg", final_sqr );
  //cvtColor(final,RGB,CV_HSV2BGR);
  RGB = imread( "sqr.jpg");
  Mat mask,mask2;
  //cvtColor(final_sqr,RGB,CV_HSV2BGR);
  inRange(RGB,good.min,good.max,mask);
  // Copia todos os pixels da imagem original que não são pretos no threshold
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
  namedWindow ("Display Grid", WINDOW_NORMAL);
  imshow("Display Grid", RGB);
  setMouseCallback("Display Grid", CallBackFunc, 0);
  waitKey(0);
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
//for(auto tmp = userdata.begin(); tmp != userdata.end(); ++tmp)
//  cout << userdata << "\n";

//printf("%f %f %f\n",good.min[0],good.min[1],good.min[2]);
  ofstream ipfile2;
  ipfile2.open("data.txt");
  ipfile2 << good.min[0]<< " " << good.min[1] << " " << good.min[2]<< "\n";
  ipfile2 << good.max[0]<< " " << good.max[1] << " " << good.max[2];
  ipfile2.close();

/********END TESTING************/
  imwrite( "sqr.jpg", final_sqr );
  imwrite( "rgb.jpg", RGB );
  imwrite( "MASK.jpg", mask );
  imwrite( "MASK2.jpg", mask2 );
  imwrite( "threshold.jpg", threshold );
  imwrite( "dst.jpg", dst );
  imwrite( "HSV.jpg", src );
  imwrite( "final.jpg", final );

  
  imwrite( "grid.jpg", final_sqr );

  //imshow( "source", src );
  //imshow( "threshold", threshold );
  //imshow( "dst", dst );
  printf("%d\n", sqr_col);
  printf("%d\n", sqr_row);
  return 0;
}