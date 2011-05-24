#!/bin/python

import os
import pyexiv2
import sqlite3
import sys
import xml.etree.ElementTree as xml

class ExifInfo:
  def __init__(self, image_path):
    # TODO: error check for image_path
    self.image_path = image_path
    self.data = {}
    self.extract_data()
  
  def extract_data(self):
    metadata = pyexiv2.ImageMetadata(self.image_path)
    metadata.read()
    exif_keys = metadata.exif_keys

    # camera manufacturer
    if "Exif.Image.Make" in exif_keys:
      self.data["camera_make"] = metadata["Exif.Image.Make"].value

    # camera model
    if "Exif.Image.Model" in exif_keys:
      self.data["camera_model"] = metadata["Exif.Image.Model"].value

    # datetime when photo was taken
    if "Exif.Photo.DateTimeOriginal" in exif_keys:
      self.data['datetime'] = metadata['Exif.Photo.DateTimeOriginal'].raw_value

    # shutter speed
    if "Exif.Photo.ShutterSpeedValue" in exif_keys:
      self.data['shutter_speed'] = \
          metadata['Exif.Photo.ShutterSpeedValue'].raw_value

    # focal length
    if "Exif.Photo.FocalLength" in exif_keys:
      self.data['focal_length'] = metadata['Exif.Photo.FocalLength'].raw_value
  
    # get gps info
    if ("Exif.GPSInfo.GPSLatitude" in exif_keys
      and "Exif.GPSInfo.GPSLongitude" in exif_keys
      and "Exif.GPSInfo.GPSLongitudeRef" in exif_keys
      and "Exif.GPSInfo.GPSLatitudeRef" in exif_keys):

      # convert rational values into decimal
      lat = self.rational_coord_to_decimal(
          metadata["Exif.GPSInfo.GPSLatitude"].value)
      lng = self.rational_coord_to_decimal(
          metadata["Exif.GPSInfo.GPSLongitude"].value)

      if (metadata["Exif.GPSInfo.GPSLatitudeRef"] == "S"):
        self.data['gps_lat'] = -lat
      else:
        self.data['gps_lat'] = lat

      if (metadata["Exif.GPSInfo.GPSLongitudeRef"] == "W"):
        self.data['gps_long'] = -lng
      else:
        self.data['gps_long'] = lng

  def rational_coord_to_decimal(self, value):
    return (float(value[0].numerator)/value[0].denominator +
            float(value[1].numerator)/value[1].denominator/60 +
            float(value[2].numerator)/value[2].denominator/3600)

class DBHelper:
  def __init__(self, db_path):
    # TODO: error check for db_path
    self.db_path = db_path
    self.conn = sqlite3.connect(self.db_path)

  def get_cursor(self):
    return self.conn.cursor()

  def commit(self):
    self.conn.commit()
  
  def clear_tables(self):
    c = self.get_cursor()
    c.execute('drop table if exists imagedata')
    self.commit()
    c.close()
    
  def create_tables(self):
    c = self.get_cursor()
    c.execute("""create table imagedata \
              (imageid integer, path text, \
              camera_make text, camera_model text, \
              datetime text, shutter_speed text, focal_length text, \
              gps_lat real, gps_long real);""")
    self.commit()
    c.close()

  def insert_from_dict(self, tablename, d):
    fields = d.keys()
    values = d.values()
    c = self.get_cursor()
    field_list = ','.join(fields)
    q_list = ','.join(['?']*len(fields))
    stmt = 'insert into %s(%s) values (%s)' % (tablename, field_list, q_list)
    c.execute(stmt, values)
    self.commit()
    c.close()
              
def main(args):
  db_helper = DBHelper('ivl.db')
  db_helper.clear_tables()
  db_helper.create_tables()
#  path = 'photo.jpg'
#  m = ExifInfo(path).data
#  db_helper.insert_from_dict('imagedata', m)

  # find the library
  settings_xml_path = '../settings.xml'
  settings_xml = xml.parse(settings_xml_path)
  lib_xml_path = settings_xml.find('library').get('path')
  lib_xml = xml.parse(os.path.dirname(settings_xml_path) + '/' + lib_xml_path)


if __name__ == "__main__":
  main(sys.argv[1:])
