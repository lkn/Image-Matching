#!/bin/python

import os
import sqlite3
import sys
from ExifInfo import ExifInfo
import xml.etree.ElementTree as xml

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
    c.execute('DROP TABLE IF EXISTS imagedata')
    self.commit()
    c.close()
    
  def create_tables(self):
    c = self.get_cursor()
    c.execute("""CREATE TABLE imagedata ( \
                    id INTEGER PRIMARY KEY, \
                    name TEXT, \
                    path TEXT, \
                    description TEXT, \
                    camera_make TEXT, \
                    camera_model TEXT, \
                    datetime TEXT, \
                    shutter_speed TEXT, \
                    focal_length TEXT, \
                    gps_lat REAL, \
                    gps_long REAL
                  );""")
    self.commit()
    c.close()

  def insert_from_dict(self, tablename, d):
    fields = d.keys()
    values = d.values()
    c = self.get_cursor()
    field_list = ','.join(fields)
    q_list = ','.join(['?']*len(fields))
    stmt = 'INSERT INTO %s(%s) VALUES (%s)' % (tablename, field_list, q_list)
    c.execute(stmt, values)
    self.commit()
    c.close()

  def populate_from_xml(self, lib_xml_path):
    # parse the library xml
    root_path = os.path.dirname(lib_xml_path) + '/'
    lib_xml = xml.parse(lib_xml_path)
    image_elements = lib_xml.getroot().findall('image')

    for lmnt in image_elements:
      data = {}
      # get the user defined data
      data['path'] = root_path + lmnt.get('path')
      data['name'] = lmnt.get('name')
      data['description'] = lmnt.text

      # get the exif data
      m = ExifInfo(data['path']).data
      for k, v in m.items():
        data[k] = v

      self.insert_from_dict('imagedata', data)


def get_lib_xml_path(settings_xml_path):
  # find the library xml
  root_path = os.path.dirname(settings_xml_path) + '/'
  settings_xml = xml.parse(settings_xml_path)
  lib_xml_path = settings_xml.find('library').get('path')
  return root_path + lib_xml_path

              
def main(args):
  db_helper = DBHelper('ivl.db')
  db_helper.clear_tables()
  db_helper.create_tables()

  lib_xml_path = get_lib_xml_path('../settings.xml')
  db_helper.populate_from_xml(lib_xml_path)


if __name__ == "__main__":
  main(sys.argv[1:])
