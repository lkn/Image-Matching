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

  def populate_from_xml(self, exe_home, lib_dir, xml_name):
    # parse the library xml
    lib_path = os.path.dirname(lib_dir) + '/'  # relative to executable
    lib_home = exe_home + lib_dir  # relative to self
    lib_xml = xml.parse(lib_home + xml_name)
    image_elements = lib_xml.getroot().findall('image')

    for lmnt in image_elements:
      data = {}
      # get the user defined data
      data['path'] = lib_dir + lmnt.get('path')
      data['name'] = lmnt.get('name')
      data['description'] = lmnt.text

      # get the exif data (need to make path relative to this script)
      m = ExifInfo(exe_home + data['path']).data
      for k, v in m.items():
        data[k] = v

      self.insert_from_dict('imagedata', data)


def get_lib_xml_path(settings_xml_path):
  """Find the library xml from the settings xml.
     exe_home is the project root path.
  """
  exe_home = os.path.dirname(settings_xml_path) + '/'
  settings_xml = xml.parse(settings_xml_path)
  lib_xml_path = settings_xml.find('library').get('path')
  (lib_dir, xml_name) = os.path.split(lib_xml_path)
  return (exe_home, lib_dir + '/', xml_name)

              
def main(args):
  db_helper = DBHelper('ivl.db')
  db_helper.clear_tables()
  db_helper.create_tables()

  (exe_home, lib_xml_dir, lib_xml_name) = get_lib_xml_path('../settings.xml')
  db_helper.populate_from_xml(exe_home, lib_xml_dir, lib_xml_name)


if __name__ == "__main__":
  main(sys.argv[1:])
