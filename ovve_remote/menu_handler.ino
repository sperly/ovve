/*  Copyright (c) 2013 Eric Lind  
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */

void menuInit(){
  mnuSet.add_menu(&mnuMode);
  mnuMode.add_item(&mitModeColorWheel, &selModeColorWheel);
  mnuMode.add_item(&mitModeSparkle, &selModeSparkle);
  mnuMode.add_item(&mitModeBassTrigger, &selModeBassTrigger);
  mnuSet.add_menu(&mnuSettings);
  //Menu mnuSettings("Settings");
  mnuSettings.add_item(&mitSettingsSave, &selSettingsSave);
  mnuSettings.add_item(&mitSettingsDefault, &selSettingsDefault);
  ms.set_root_menu(&mnuSet);
}

void selModeColorWheel(MenuItem* p_menu_item){
  lcd.setCursor(0, 2);
  lcd.print("0 - Colorwheel");
  config_data.mode = 0;
  MenuExc = true;
  MenuFwd = false;
  lastAction = 0;
  //SendMode(0);
}

void selModeSparkle(MenuItem* p_menu_item){
  lcd.setCursor(0, 2);
  lcd.print("1 - Sparkle");
  config_data.mode = 1;
  MenuExc = true;
  MenuFwd = false;
  lastAction = 0;
  //SendMode(1);
}

void selModeBassTrigger(MenuItem* p_menu_item){
  MenuExc = true;
  MenuFwd = false;
}

void selSettingsSave(MenuItem* p_menu_item){
  writeConfig();
  lcd.setCursor(0, 2);
  lcd.print("Config written!");
  MenuExc = true;
  MenuFwd = false;
  lastAction = 0;
}

void selSettingsDefault(MenuItem* p_menu_item){
  defaultConfig();
  lcd.setCursor(0, 2);
  lcd.print("Default set!");
  MenuExc = true;
  MenuFwd = false;;
}
