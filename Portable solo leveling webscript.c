function doGet(e) {
  return handleRequest(e);
}

function doPost(e) {
  return handleRequest(e);
}

function handleRequest(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("Sheet1");
  var user = e.parameter.user;
  var action = e.parameter.action;
  var exp = e.parameter.exp;
  
  // Find user row
  var data = sheet.getDataRange().getValues();
  var row = -1;
  for (var i = 1; i < data.length; i++) {
    if (data[i][0] == user) {
      row = i + 1;
      break;
    }
  }
  
  if (action == "get") {
    if (row == -1) return ContentService.createTextOutput("0");
    return ContentService.createTextOutput(sheet.getRange(row, 2).getValue());
  }
  else if (action == "update") {
    if (row == -1) {
      sheet.appendRow([user, parseInt(exp)]);
    } else {
      sheet.getRange(row, 2).setValue(parseInt(exp));
    }
    return ContentService.createTextOutput("OK");
  }
}
