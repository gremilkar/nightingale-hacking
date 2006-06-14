/*
 //
// BEGIN SONGBIRD GPL
// 
// This file is part of the Songbird web player.
//
// Copyright� 2006 Pioneers of the Inevitable LLC
// http://songbirdnest.com
// 
// This file may be licensed under the terms of of the
// GNU General Public License Version 2 (the �GPL�).
// 
// Software distributed under the License is distributed 
// on an �AS IS� basis, WITHOUT WARRANTY OF ANY KIND, either 
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

//
// sbIPlaylistReader Object (RSS)
//

const SONGBIRD_PLAYLISTRDF_CONTRACTID = "@songbird.org/Songbird/Playlist/Reader/RDF;1";
const SONGBIRD_PLAYLISTRDF_CLASSNAME = "Songbird RDF Playlist Interface";
const SONGBIRD_PLAYLISTRDF_CID = Components.ID("{5778E746-3435-4f7e-9201-99898E9A916A}");
const SONGBIRD_PLAYLISTRDF_IID = Components.interfaces.sbIPlaylistReader;

function CPlaylistRDF()
{
  jsLoader = Components.classes["@mozilla.org/moz/jssubscript-loader;1"].getService(Components.interfaces.mozIJSSubScriptLoader);
  jsLoader.loadSubScript( "chrome://songbird/content/scripts/songbird_interfaces.js", this );
}

/* I actually need a constructor in this case. */
CPlaylistRDF.prototype.constructor = CPlaylistRDF;

/* the CPlaylistRDF class def */
CPlaylistRDF.prototype = 
{
  originalURL: "",
  
  m_document: null,
  m_playlistmgr: null,
  m_playlist: null,
  m_library: null,
  m_query: null,
  
  m_guid: "",
  m_table: "",
  m_append: false,
  
  Read: function( strURL, strGUID, strDestTable, bAppendOrReplace, /* out */ errorCode )
  {
    try
    {
      errorCode.value = 0;

      var domParser = Components.classes["@mozilla.org/xmlextras/domparser;1"].createInstance(Components.interfaces.nsIDOMParser);
      var pFileReader = (Components.classes["@mozilla.org/network/file-input-stream;1"]).createInstance(Components.interfaces.nsIFileInputStream);
      var pFile = (Components.classes["@mozilla.org/file/local;1"]).createInstance(Components.interfaces.nsILocalFile);
      var pURI = (Components.classes["@mozilla.org/network/simple-uri;1"]).createInstance(Components.interfaces.nsIURI);

      if ( domParser && pFile && pURI && pFileReader )
      {
        pURI.spec = strURL;
        if ( pURI.scheme == "file" )
        {
          var cstrPathToPLS = pURI.path;
          cstrPathToPLS = cstrPathToPLS.substr( 3, cstrPathToPLS.length );
          pFile.initWithPath(cstrPathToPLS);
          if ( pFile.isFile() )
          {
            pFileReader.init( pFile, /* PR_RDONLY */ 1, 0, /*nsIFileInputStream::CLOSE_ON_EOF*/ 0 );
            var document = domParser.parseFromStream(pFileReader, null, pFileReader.available(), "application/xml");
            pFileReader.close();
            
            if(document)
            {
              const MediaLibrary = new Components.Constructor("@songbird.org/Songbird/MediaLibrary;1", "sbIMediaLibrary");
              const PlaylistManager = new Components.Constructor("@songbird.org/Songbird/PlaylistManager;1", "sbIPlaylistManager");

              this.m_guid = strGUID;
              this.m_table = strDestTable;
              this.m_append = bAppendOrReplace;
              
              this.m_query = new this.sbIDatabaseQuery();
              this.m_query.SetAsyncQuery(true);
              this.m_query.SetDatabaseGUID(strGUID);

              this.m_document = document;
              this.m_library = new MediaLibrary();
              this.m_playlistmgr = new PlaylistManager();
              
              this.m_library.SetQueryObject(this.m_query);
              var playlist = this.m_playlistmgr.GetPlaylist(strDestTable, this.m_query);
              
              if(playlist)
              {
                this.m_playlist = playlist;
                return this.ProcessXMLDocument(this.m_document);
              }
            }
          }
        }
      }
    }
    catch ( err )
    {
      throw "CPlaylistRDF::Read - " + err;
    }
    
    return false;
  },
  
  Vote: function( strURL )
  {
    try
    {
      return 10000;
    }
    catch ( err )
    {
      throw "CPlaylistRDF::Vote - " + err;
    }
  },
  
  Name: function()
  {
    try
    {
      return "RDF Playlist"; // ????
    }
    catch ( err )
    {
      throw "CPlaylistRDF::Name - " + err;
    }
  },
  
  Description: function()
  {
    try
    {
      return "RDF Playlist"; // ????
    }
    catch ( err )
    {
      throw "CPlaylistRDF::Description - " + err;
    }
  },
  
  SupportedMIMETypes: function( /* out */ nMIMECount )
  {
    var retval = new Array;
    nMIMECount.value = 0;
    
    try
    {
      retval.push( "text/xml" );
      retval.push( "text/rdf+xml" );
      retval.push( "application/xml" );
      nMIMECount.value = retval.length;
    }
    catch ( err )
    {
      throw "CPlaylistRDF::SupportedMIMETypes - " + err;
    }
    return retval;
  },
  
  SupportedFileExtensions: function( /* out */ nExtCount )
  {
    var retval = new Array;
    nExtCount.value = 0;
    
    try
    {
      retval.push( "rdf" );
      retval.push( "xml" );
      
      nExtCount.value = retval.length;
    }
    catch ( err )
    {
      throw "CPlaylistRDF::SupportedFileExtensions - " + err;
    }
    
    return retval;
  },
  
  ProcessXMLDocument: function( xmlDocument )
  {
    var ret = false;
    try
    {
      var document = xmlDocument.documentElement;
      dump("CPlaylistRDF::ProcessXMLDocument - Tag Name: " + document.tagName + "\n");
      
      if(document.tagName == "rdf:RDF")
      {
        dump("CPlaylistRDF::ProcessXMLDocument - RSS/RDF Version: 1.0\n");
        ret = this.ProcessRDFFeed(document);
      }
    }      
    catch(err)
    {
      throw "CPlaylistRDF::ProcessXMLDocument - " + err;
    }
    
    return ret;
  },
  
  ProcessRDFFeed: function( xmlDocument )
  {
    try
    {
      
    }
    catch(err)
    {
      throw "CPlaylistRDF::ProcessXMLDocument - " + err;
    }
    
    return false;
  },
  
  IsMediaUrl: function( the_url )
  {
    if ( ( the_url.indexOf ) && 
          (
            // Protocols at the beginning
            ( the_url.indexOf( "mms:" ) == 0 ) || 
            ( the_url.indexOf( "rtsp:" ) == 0 ) || 
            // File extensions at the end
            ( the_url.indexOf( ".pls" ) != -1 ) || 
            ( the_url.indexOf( "rss" ) != -1 ) || 
            ( the_url.indexOf( ".m3u" ) == ( the_url.length - 4 ) ) || 
//            ( the_url.indexOf( ".rm" ) == ( the_url.length - 3 ) ) || 
//            ( the_url.indexOf( ".ram" ) == ( the_url.length - 4 ) ) || 
//            ( the_url.indexOf( ".smil" ) == ( the_url.length - 5 ) ) || 
            ( the_url.indexOf( ".mp3" ) == ( the_url.length - 4 ) ) ||
            ( the_url.indexOf( ".ogg" ) == ( the_url.length - 4 ) ) ||
            ( the_url.indexOf( ".flac" ) == ( the_url.length - 5 ) ) ||
            ( the_url.indexOf( ".wav" ) == ( the_url.length - 4 ) ) ||
            ( the_url.indexOf( ".m4a" ) == ( the_url.length - 4 ) ) ||
            ( the_url.indexOf( ".wma" ) == ( the_url.length - 4 ) ) ||
            ( the_url.indexOf( ".wmv" ) == ( the_url.length - 4 ) ) ||
            ( the_url.indexOf( ".asx" ) == ( the_url.length - 4 ) ) ||
            ( the_url.indexOf( ".asf" ) == ( the_url.length - 4 ) ) ||
            ( the_url.indexOf( ".avi" ) == ( the_url.length - 4 ) ) ||
            ( the_url.indexOf( ".mov" ) == ( the_url.length - 4 ) ) ||
            ( the_url.indexOf( ".mpg" ) == ( the_url.length - 4 ) ) ||
            ( the_url.indexOf( ".mp4" ) == ( the_url.length - 4 ) )
          )
        )
    {
      return true;
    }
    return false;
  },
  
  ConvertUrlToDisplayName: function( url )
  {
    url = decodeURI( url );
    // Set the title display  
    var the_value = "";
    if ( url.lastIndexOf('/') != -1 )
    {
      the_value = url.substring( url.lastIndexOf('/') + 1, url.length );
    }
    else if ( url.lastIndexOf('\\') != -1 )
    {
      the_value = url.substring( url.lastIndexOf('\\') + 1, url.length );
    }
    else
    {
      the_value = url;
    }
    if ( the_value.length == 0 )
    {
      the_value = url;
    }
    return the_value;
  },

  QueryInterface: function(iid)
  {
      if (!iid.equals(Components.interfaces.nsISupports) &&
          !iid.equals(SONGBIRD_PLAYLISTRDF_IID))
          throw Components.results.NS_ERROR_NO_INTERFACE;
      return this;
  }
}; //CPlaylistRDF

/**
 * \class sbPlaylistRDFModule
 * \brief 
 */
var sbPlaylistRDFModule = 
{
  registerSelf: function(compMgr, fileSpec, location, type)
  {
    compMgr = compMgr.QueryInterface(Components.interfaces.nsIComponentRegistrar);
    compMgr.registerFactoryLocation(SONGBIRD_PLAYLISTRDF_CID, 
                                    SONGBIRD_PLAYLISTRDF_CLASSNAME, 
                                    SONGBIRD_PLAYLISTRDF_CONTRACTID, 
                                    fileSpec, 
                                    location,
                                    type);
  },

  getClassObject: function(compMgr, cid, iid) 
  {
    if(!cid.equals(SONGBIRD_PLAYLISTRDF_CID))
        throw Components.results.NS_ERROR_NO_INTERFACE;

    if(!iid.equals(Components.interfaces.nsIFactory))
        throw Components.results.NS_ERROR_NOT_IMPLEMENTED;

    return sbPlaylistRDFFactory;
  },

  canUnload: function(compMgr)
  { 
    return true; 
  }
}; //sbPlaylistRDFModule

/**
 * \class sbPlaylistRDFFactory
 * \brief 
 */
var sbPlaylistRDFFactory =
{
  createInstance: function(outer, iid)
  {
    if (outer != null)
        throw Components.results.NS_ERROR_NO_AGGREGATION;

    if (!iid.equals(SONGBIRD_PLAYLISTRDF_IID) &&
        !iid.equals(Components.interfaces.nsISupports))
        throw Components.results.NS_ERROR_INVALID_ARG;

    return (new CPlaylistRDF()).QueryInterface(iid);
  }
}; //sbPlaylistRDFFactory

/**
 * \function NSGetModule
 * \brief 
 */
function NSGetModule(comMgr, fileSpec)
{ 
  return sbPlaylistRDFModule;
} //NSGetModule
