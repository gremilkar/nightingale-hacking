/*
//
// BEGIN SONGBIRD GPL
//
// This file is part of the Songbird web player.
//
// Copyright(c) 2005-2008 POTI, Inc.
// http://songbirdnest.com
//
// This file may be licensed under the terms of of the
// GNU General Public License Version 2 (the "GPL").
//
// Software distributed under the License is distributed
// on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either
// express or implied. See the GPL for the specific language
// governing rights and limitations.
//
// You should have received a copy of the GPL along with this
// program. If not, go to http://www.gnu.org/licenses/gpl.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
// END SONGBIRD GPL
//
*/

/**
 * \brief Test file
 */

function runTest () {

  var dbq = Cc["@songbirdnest.com/Songbird/DatabaseQuery;1"]
              .createInstance(Ci.sbIDatabaseQuery);

  var ios = Cc["@mozilla.org/network/io-service;1"]
              .createInstance(Ci.nsIIOService);
  
  var dir = Cc["@mozilla.org/file/directory_service;1"]
              .createInstance(Ci.nsIProperties);
              
  var testdir = dir.get("ProfD", Ci.nsIFile);
  
  var actualdir = testdir.clone();
  actualdir.append("db_tests");
  
  if(!actualdir.exists())
  {
    try {
      actualdir.create(Ci.nsIFile.DIRECTORY_TYPE, 0700);
    } catch(e) {
      //Some failures might be handled later. Some might be ignored.
      throw e;
    }
  }
  
  var uri = ios.newFileURI(actualdir);
  dbq.databaseLocation = uri;
  
  assertEqual(dbq.databaseLocation.spec, uri.spec);

  // clear the table first and then re-create
  dbq.setDatabaseGUID("test_sync");
  dbq.addQuery("drop table proxy_test");
  dbq.addQuery("create table proxy_test (name text, value text)");
  dbq.execute();
  dbq.waitForCompletion();
  dbq.resetQuery();

  for ( var j = 0; j < 500; j++ ) {
    log("Looping: doing query number: " + j);
    var query = Cc["@songbirdnest.com/Songbird/DatabaseQuery;1"]
                .createInstance(Ci.sbIDatabaseQuery);
    query.databaseLocation = uri;
    query.setDatabaseGUID("test_sync");

    for ( var i = 0; i < 10; i++ ) {
      query.addQuery("insert into proxy_test values ('test " + j + i + "', 'testing... " + j + i + "')");
    }
  
    query.execute();
  }

  return Components.results.NS_OK;
}

