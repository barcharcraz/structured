/*
 *  GXML/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000 Bruno Levy
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     levy@loria.fr
 *
 *     ISA Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 */
 
#include <OGF/image/io/image_serializer_png.h>
#include <OGF/image/types/image.h>
#include <png.h>

namespace OGF {

    Image* ImageSerializer_png::serialize_read(const std::string& file_name) {

        FILE* in = fopen(file_name.c_str(), "rb") ;
        if(in == nil) {
            Logger::err("ImageSerializer") << "Could not open file: \'"
                                           << file_name
                                           << "\'" << std::endl ;
            return nil ;
        }

        png_structp png_ptr = png_create_read_struct(
            PNG_LIBPNG_VER_STRING,
            (png_voidp) NULL, (png_error_ptr) NULL, (png_error_ptr) NULL 
        ) ;
        if ( png_ptr == nil ) {
            fclose(in) ;
            return nil ;
        } 
        
        png_infop info_ptr = png_create_info_struct ( png_ptr );
        if ( info_ptr == nil ) {
            png_destroy_read_struct (
                &png_ptr, (png_infopp)NULL, (png_infopp)NULL 
            ) ;
            fclose(in) ;
            return nil ;
        }

        png_init_io ( png_ptr, in );

        // read header 
        int bit_depth, color_type, interlace_type ;
        png_uint_32 width, height;
        png_read_info ( png_ptr, info_ptr );
        png_get_IHDR ( 
            png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
            &interlace_type, NULL, NULL 
        ) ;


        Image* result = nil;



        if(color_type == PNG_COLOR_TYPE_GRAY) {
            result = new Image(Image::GRAY, width, height) ;
        } else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
            result = new Image(Image::RGBA, width, height) ;
        } else {
            result = new Image(Image::RGB, width, height) ;
        }
        
        // If colormapped, convert to RGB
        // TODO: create a ColorMapped Image
        if ( 
            color_type == PNG_COLOR_TYPE_PALETTE  
        ) {
            png_set_expand(png_ptr) ;
        }

        // Ignore alpha (for the moment)
        // TODO: read alpha if present
        //png_set_strip_alpha(png_ptr) ;

        // Read the image one line at a time
        png_bytep row_pointer = (png_bytep) png_malloc ( 
            png_ptr, png_get_rowbytes ( png_ptr, info_ptr ) 
        ) ;
        row_pointer = (png_bytep) malloc ( result->width() * 4 );
        for ( unsigned int row = 0; row < height; row++ ) {
            png_read_rows ( png_ptr, &row_pointer, NULL, 1 );
            if(
                color_type == PNG_COLOR_TYPE_GRAY ||
                color_type == PNG_COLOR_TYPE_PALETTE
            ) {
                memcpy ( 
                    result->base_mem() + row * result->width(),
                    row_pointer,  result->width() 
                ) ;
            } else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
                memcpy ( 
                    result->base_mem() + row * result->width() * 4,
                    row_pointer,  result->width() * 4
                ) ;
			} else {
                memcpy ( 
                    result->base_mem() + row * result->width() * 3,
                    row_pointer,  result->width() * 3  
                ) ;
			}
        }
        free ( row_pointer );

        png_read_end ( png_ptr, info_ptr );
        png_destroy_read_struct ( &png_ptr, &info_ptr, (png_infopp)NULL );
        
        fclose(in) ;
        return result ;
    }
    
    bool ImageSerializer_png::read_supported() const {
        return true ;
    }

    bool ImageSerializer_png::serialize_write(
        const std::string& file_name, const Image* image
    ) {
        if(
            image->color_encoding() != Image::RGB && 
            image->color_encoding() != Image::RGBA  &&
            image->color_encoding() != Image::GRAY
        ) {
            Logger::err("ImageSerializer_png") 
                << "PNG writing only supported for GRAY, RGB and RGBA color encoding"
                << ", sorry" << std::endl ;
            return false ;
        }
        
        FILE* out = fopen(file_name.c_str(), "wb") ;
        if(out == nil) {
            Logger::err("ImageSerializer") << "Could not open file: \'"
                                           << file_name
                                           << "\'" << std::endl ;
            return false ;
        }

        png_structp png_ptr = png_create_write_struct(
            PNG_LIBPNG_VER_STRING,
            (png_voidp) NULL, (png_error_ptr) NULL, (png_error_ptr) NULL 
        ) ;
        if ( png_ptr == nil ) {
            fclose(out) ;
            return false ;
        } 

        png_infop info_ptr = png_create_info_struct ( png_ptr );
        png_init_io( png_ptr, out );


        png_byte png_color_encoding ;

        switch(image->color_encoding()) {
        case Image::GRAY:
            png_color_encoding = PNG_COLOR_TYPE_GRAY ;
            break ;
        case Image::RGB : 
            png_color_encoding = PNG_COLOR_TYPE_RGB ;
            break ;
        case Image::RGBA : 
            png_color_encoding = PNG_COLOR_TYPE_RGBA ;
            break ;
        default:
            ogf_assert_not_reached ;
            break ;
        }

        png_set_IHDR ( 
            png_ptr, info_ptr, image->width(), image->height(),
            8, png_color_encoding, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE 
        );

        png_text comment;
        comment.compression = PNG_TEXT_COMPRESSION_NONE;
        comment.key = "Comment" ;
        comment.text = "Made with Graphite" ;
        png_set_text ( png_ptr, info_ptr, &comment, 1 );

        png_write_info ( png_ptr, info_ptr );
        for (int row = 0; row < image->height(); row++ ) {
            png_write_row ( 
                png_ptr, 
                image->base_mem() + image->width() * row * image->bytes_per_pixel()
            );
        }
        png_write_end ( png_ptr, info_ptr );
        fflush ( out );
        png_destroy_write_struct ( &png_ptr, (png_infopp)NULL );
        fclose(out) ;

        return true ;
    }
    
    bool ImageSerializer_png::write_supported() const {
        return true ;
    }
    
    bool ImageSerializer_png::streams_supported() const {
        return false ;
    }

    bool ImageSerializer_png::binary() const {
        return true ;
    }
}
