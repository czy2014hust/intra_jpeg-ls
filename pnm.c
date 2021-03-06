/*************************************************************************\
 * JPEG-LS Lossless image encoder/decoder                                *
 * Copyright (C) 2010 Davide Bardone <davide.bardone@gmail.com>          *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
\*************************************************************************/

#include "pnm.h"


void free_image_mem(uint16*** image, uint16 height, uint8 n_of_components)
{
    uint8 c;
	uint16 row;
        for(c=0; c<n_of_components; c++)
	{
                for(row=0; row<height; row++)
			free(image[c][row]);
		free(image[c]);
	}
	free(image);
}


void free_image_data(image_data* im_data)
{
        free_image_mem(im_data->image, im_data->height, im_data->n_comp);
        free(im_data);
}


image_data* load_image(char* image_name)
{
  printf("%s\n", image_name);
	FILE *fp;
	uint16*** image;
	uint8 magic_number=0;
	uint8 n_of_components;
	uint8 bytes_to_read;
	uint16 width, height, maxval;
	char head_buffer[HEAD_BUFF_LEN];
	uint16 c1,c2,c3;
	width=height=maxval=0;
	uint8 c;
	uint16 row,col;
	image_data* im_data;

	/*open image file*/
	fp=fopen(image_name, "r");
	if(fp==NULL)
	{
		fprintf(stderr, "Impossible to open file %s\n", image_name);
		exit(EXIT_FAILURE);
	}

	/*parsing header*/

	/* at the moment only PPM file format is supported */

	/* TODO better header parsing */

	fgets(head_buffer,HEAD_BUFF_LEN,fp);

        printf("reading image header done!\n");
  //fclose(fp);
  //exit(EXIT_FAILURE);
	if(head_buffer[0]=='P' && magic_number==0)
		if(head_buffer[1]=='3')
			magic_number=3;
		else if(head_buffer[1]=='6')
		{

            magic_number=6;

            width = (head_buffer[3] - '0') * 1000 + (head_buffer[4] - '0') * 100 +
				(head_buffer[5] - '0') * 10 + (head_buffer[6] - '0') * 1;
		    height =(head_buffer[8] - '0') * 1000 + (head_buffer[9] - '0') * 100 +
				(head_buffer[10] - '0') * 10 + (head_buffer[11] - '0') * 1;
		    maxval =(head_buffer[13] - '0') * 100 + (head_buffer[14] - '0') * 10 +
				(head_buffer[15] - '0') * 1 ;
            printf("width is %d and height is %d\nmaxval is %d\n", width, height, maxval);
		}
		else
            printf("Damn");
    else
        printf("image format is not right!");


	/*image data allocation*/

	if ((magic_number==3)||(magic_number==6))
		n_of_components = PPM_COMPONENTS;

	image = (uint16***)malloc(n_of_components*sizeof(uint16**));
	if(image==NULL)
	{
		fprintf(stderr, "Error in memory allocation\n");
		fclose(fp);
		exit(EXIT_FAILURE);
	}

	for(c=0; c<n_of_components; c++)
	{
		image[c] = (uint16**)malloc(height*sizeof(uint16*));
		if(image[c]==NULL)
		{
                	fprintf(stderr, "Error in memory allocation\n");
                	fclose(fp);
			exit(EXIT_FAILURE);
        	}
		for(row=0; row<height; row++)
		{
			image[c][row] = (uint16*)malloc(width*sizeof(uint16));
			if(image[c][row]==NULL)
			{
                		fprintf(stderr, "Error in memory allocation\n");
                		fclose(fp);
				exit(EXIT_FAILURE);
        		}
		}
	}

  for(int z=0;z<3;z++)
    for(int y=0;y<1024;y++)
      memset(image[z][y], 0x0000 , 1024*2);

	/*read image data*/

	//bytes_to_read = (log2(maxval)<256) ? 1 : 2;
	bytes_to_read = 1;
	if(magic_number==3)
		for(row=0; row<height; row++)
			for(col=0; col<width; col++)
				if(fscanf(fp,"%hu %hu %hu",&c1,&c2,&c3)==3){
					image[0][row][col] = c1;
					image[1][row][col] = c2;
					image[2][row][col] = c3;
				}
				else
				{
          fprintf(stderr, "%s data is incomplete.\n", image_name);
          free_image_mem(image, height, n_of_components);
					fclose(fp);
					exit(EXIT_FAILURE);
		                }
	else if(magic_number==6)
		for(row=0; row<height; row++)
      for(col=0; col<width; col++)
			{
				for(c=0; c<n_of_components; c++)
				{
					if(fread(&image[c][row][col],bytes_to_read,1,fp)!=bytes_to_read)
					{
		        fprintf(stderr, "%s data is incomplete.\n", image_name);
		        free_image_mem(image, height, n_of_components);
						fclose(fp);
						exit(EXIT_FAILURE);
					}
          //if((row==0) && (col < 5))
            //printf("%d\n",image[0][0][col]);
          }
			}

	fclose(fp);

	im_data = (image_data*)malloc(sizeof(image_data));
	if(im_data==NULL)
	{
    fprintf(stderr, "Error in memory allocation\n");
    fclose(fp);
    free_image_mem(image, height, n_of_components);
		exit(EXIT_FAILURE);
        }
	im_data->width = width;
	im_data->height = height;
	im_data->maxval = maxval;
	im_data->image = image;
	im_data->n_comp = n_of_components;
    printf("reload image done!\n");
	return im_data;
}


image_data* allocate_image_data()
{
	image_data* im_data = (image_data*)malloc(sizeof(image_data));
	if(im_data==NULL)
	{
                fprintf(stderr, "Error in memory allocation\n");
		exit(EXIT_FAILURE);
        }
	return im_data;
}


void allocate_image(image_data* im_data)
{
	uint8 c;
	uint16 row;

	im_data->image = (uint16***)malloc(im_data->n_comp*sizeof(uint16**));
	if(im_data->image==NULL)
	{
		fprintf(stderr, "Error in memory allocation\n");
		exit(EXIT_FAILURE);
	}

	for(c=0; c<im_data->n_comp; c++)
	{
		im_data->image[c] = (uint16**)malloc(im_data->height*sizeof(uint16*));
		if(im_data->image[c]==NULL)
		{
                	fprintf(stderr, "Error in memory allocation\n");
			exit(EXIT_FAILURE);
        	}
		for(row=0; row<im_data->height; row++)
		{
			im_data->image[c][row] = (uint16*)malloc(im_data->width*sizeof(uint16));
			if(im_data->image[c][row]==NULL)
			{
                		fprintf(stderr, "Error in memory allocation\n");
				exit(EXIT_FAILURE);
        		}
		}
	}
}

void write_image(char* image_name, image_data* im_data)
{
	FILE* fp;
	uint16 row,col;
	uint16*** image;
	uint8 bytes_to_write;

	fp = fopen(image_name,"w");
	if(fp==NULL){
                fprintf(stderr, "Error creating file %s\n", image_name);
                free_image_data(im_data);
		exit(EXIT_FAILURE);
        }

	bytes_to_write = (log2(im_data->maxval)<256) ? 1 : 2;

	if(im_data->n_comp==3)
	{
		/* output file is a PPM file with magic number P6 */

		fprintf(fp,"P6 ");
		//fprintf(fp,"#JPEG-LS decoder\n");
		fprintf(fp,"%hu %hu ",im_data->width,im_data->height);
		fprintf(fp,"%hu\n",im_data->maxval);

		image=im_data->image;

		for(row=0;row<im_data->height;row++)
			for(col=0;col<im_data->width;col++)
			{
				fwrite(&image[0][row][col],bytes_to_write,1,fp);
				fwrite(&image[1][row][col],bytes_to_write,1,fp);
				fwrite(&image[2][row][col],bytes_to_write,1,fp);
			}
	}

	/* TODO output file for greyscale images */

	fclose(fp);
	free_image_data(im_data);
}
