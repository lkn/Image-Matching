#!/bin/python

import os
import sqlite3
import sys
from ExifInfo import *
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
                    path TEXT, \
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
              
def main(args):
  db_helper = DBHelper('ivl.db')
  db_helper.clear_tables()
  db_helper.create_tables()
  path = 'guiness_factory_2.JPG'
  m = ExifInfo(path).data
  db_helper.insert_from_dict('imagedata', m)

  # find the library
#  settings_xml_path = '../settings.xml'
#  settings_xml = xml.parse(settings_xml_path)
#  lib_xml_path = settings_xml.find('library').get('path')
#  lib_xml = xml.parse(os.path.dirname(settings_xml_path) + '/' + lib_xml_path)


if __name__ == "__main__":
  main(sys.argv[1:])
