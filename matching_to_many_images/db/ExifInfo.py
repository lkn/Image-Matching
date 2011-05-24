#!/bin/python

import sys
import pprint
import pyexiv2

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
      dt = metadata['Exif.Photo.DateTimeOriginal'].value
      self.data['datetime'] = dt.strftime('%A %d %B %Y, %H:%M:%S')

    # shutter speed (funnily enough, its NOT ShutterSpeedValue tag)
    if "Exif.Photo.ExposureTime" in exif_keys:
      self.data['shutter_speed'] = \
          metadata['Exif.Photo.ExposureTime'].raw_value

    # focal length
    if "Exif.Photo.FocalLength" in exif_keys:
      v = metadata['Exif.Photo.FocalLength'].value
      txt = str(v.numerator / v.denominator) + ' mm'
      self.data['focal_length'] = txt
  
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

      if (metadata["Exif.GPSInfo.GPSLatitudeRef"].value == "S"):
        self.data['gps_lat'] = -lat
      else:
        self.data['gps_lat'] = lat

      if (metadata["Exif.GPSInfo.GPSLongitudeRef"].value == "W"):
        self.data['gps_long'] = -lng
      else:
        self.data['gps_long'] = lng

  def rational_coord_to_decimal(self, value):
    return (float(value[0].numerator)/value[0].denominator +
            float(value[1].numerator)/value[1].denominator/60 +
            float(value[2].numerator)/value[2].denominator/3600)

  def dump(self):
    pp = pprint.PrettyPrinter(indent = 4)
    pp.pprint(self.data)


if __name__ == "__main__":
  if len(sys.argv)< 2:
    print "Need to specify image path!"
  else:
    m = ExifInfo(sys.argv[1])
    m.dump()

