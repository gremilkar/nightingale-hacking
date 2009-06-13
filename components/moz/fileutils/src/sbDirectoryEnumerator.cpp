/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set sw=2 :miv */
/*
//
// BEGIN SONGBIRD GPL
//
// This file is part of the Songbird web player.
//
// Copyright(c) 2005-2009 POTI, Inc.
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

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
// Songbird directory enumerator.
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/**
 * \file  sbDirectoryEnumerator.cpp
 * \brief Songbird Directory Enumerator Source.
 */

//------------------------------------------------------------------------------
//
// Songbird directory enumerator imported services.
//
//------------------------------------------------------------------------------

// Self imports.
#include "sbDirectoryEnumerator.h"

// Mozilla imports.
#include <nsAutoLock.h>
#include <nsIFile.h>


//------------------------------------------------------------------------------
//
// Songbird directory enumerator nsISupports implementation.
//
//------------------------------------------------------------------------------

NS_IMPL_THREADSAFE_ISUPPORTS1(sbDirectoryEnumerator, sbIDirectoryEnumerator)


//------------------------------------------------------------------------------
//
// Songbird directory enumerator sbIDirectoryEnumerator implementation.
//
//------------------------------------------------------------------------------

/**
 * \brief Enumerate the directory specified by aDirectory.
 *
 * \param aDirectory          Directory to enumerate.
 */

NS_IMETHODIMP
sbDirectoryEnumerator::Enumerate(nsIFile* aDirectory)
{
  // Validate arguments and state.
  NS_ENSURE_ARG_POINTER(aDirectory);
  NS_PRECONDITION(mIsInitialized, "Directory enumerator not initialized");

  // Function variables.
  PRBool   success;
  nsresult rv;

  // Operate under the enumerator lock.
  nsAutoLock autoLock(mEnumeratorLock);

  // Get the entries in the directory.
  nsCOMPtr<nsISimpleEnumerator> entriesEnum;
  rv = aDirectory->GetDirectoryEntries(getter_AddRefs(entriesEnum));
  NS_ENSURE_SUCCESS(rv, rv);

  // Initialize the entries enumeration stack.
  mEntriesEnumStack.Clear();
  success = mEntriesEnumStack.AppendObject(entriesEnum);
  NS_ENSURE_TRUE(success, NS_ERROR_OUT_OF_MEMORY);

  // Scan for the next file.
  rv = ScanForNextFile();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}


/**
 * \brief Return true if more elements are available to enumerate.
 *
 * \return True if more elements are available.
 */

NS_IMETHODIMP
sbDirectoryEnumerator::HasMoreElements(PRBool* aHasMoreElements)
{
  // Validate arguments and state.
  NS_ENSURE_ARG_POINTER(aHasMoreElements);
  NS_PRECONDITION(mIsInitialized, "Directory enumerator not initialized");

  // Operate under the enumerator lock.
  nsAutoLock autoLock(mEnumeratorLock);

  // Return results.
  *aHasMoreElements = (mEntriesEnumStack.Count() > 0);

  return NS_OK;
}


/**
 * \brief Return the next file in the enumeration.
 *
 * \return Next file in enumeration.
 */

NS_IMETHODIMP
sbDirectoryEnumerator::GetNext(nsIFile** aFile)
{
  // Validate arguments and state.
  NS_ENSURE_ARG_POINTER(aFile);
  NS_PRECONDITION(mIsInitialized, "Directory enumerator not initialized");

  // Function variables.
  nsresult rv;

  // Operate under the enumerator lock.
  nsAutoLock autoLock(mEnumeratorLock);

  // Return the next file.  Return error if no next file.
  if (mNextFile)
    mNextFile.forget(aFile);
  else
    return NS_ERROR_NOT_AVAILABLE;

  // Scan for the next file.
  rv = ScanForNextFile();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}


//
// Getters/setters.
//

/**
 * \brief Maximum depth to which to enumerate.  A depth of 0 will enumerate to
 *        an unlimited depth.  A depth of 1 will enumerate only the base
 *        directory (same as getDirectoryEntries).
 */

NS_IMETHODIMP
sbDirectoryEnumerator::GetMaxDepth(PRUint32* aMaxDepth)
{
  NS_ENSURE_ARG_POINTER(aMaxDepth);
  NS_PRECONDITION(mIsInitialized, "Directory enumerator not initialized");
  nsAutoLock autoLock(mEnumeratorLock);
  *aMaxDepth = mMaxDepth;
  return NS_OK;
}

NS_IMETHODIMP
sbDirectoryEnumerator::SetMaxDepth(PRUint32 aMaxDepth)
{
  NS_PRECONDITION(mIsInitialized, "Directory enumerator not initialized");
  nsAutoLock autoLock(mEnumeratorLock);
  mMaxDepth = aMaxDepth;
  return NS_OK;
}


/**
 * \brief If true, only return directories in getNext.
 */

NS_IMETHODIMP
sbDirectoryEnumerator::GetDirectoriesOnly(PRBool* aDirectoriesOnly)
{
  NS_ENSURE_ARG_POINTER(aDirectoriesOnly);
  NS_PRECONDITION(mIsInitialized, "Directory enumerator not initialized");
  nsAutoLock autoLock(mEnumeratorLock);
  *aDirectoriesOnly = mDirectoriesOnly;
  return NS_OK;
}

NS_IMETHODIMP
sbDirectoryEnumerator::SetDirectoriesOnly(PRBool aDirectoriesOnly)
{
  NS_PRECONDITION(mIsInitialized, "Directory enumerator not initialized");
  nsAutoLock autoLock(mEnumeratorLock);
  mDirectoriesOnly = aDirectoriesOnly;
  return NS_OK;
}


/**
 * \brief If true, only return files in getNext.
 */

NS_IMETHODIMP
sbDirectoryEnumerator::GetFilesOnly(PRBool* aFilesOnly)
{
  NS_ENSURE_ARG_POINTER(aFilesOnly);
  NS_PRECONDITION(mIsInitialized, "Directory enumerator not initialized");
  nsAutoLock autoLock(mEnumeratorLock);
  *aFilesOnly = mFilesOnly;
  return NS_OK;
}

NS_IMETHODIMP
sbDirectoryEnumerator::SetFilesOnly(PRBool aFilesOnly)
{
  NS_PRECONDITION(mIsInitialized, "Directory enumerator not initialized");
  nsAutoLock autoLock(mEnumeratorLock);
  mFilesOnly = aFilesOnly;
  return NS_OK;
}


//------------------------------------------------------------------------------
//
// Songbird directory enumerator public services.
//
//------------------------------------------------------------------------------

/**
 * Construct a Songbird directory enumerator object.
 */

sbDirectoryEnumerator::sbDirectoryEnumerator() :
  mIsInitialized(PR_FALSE),
  mEnumeratorLock(nsnull),
  mMaxDepth(0),
  mDirectoriesOnly(PR_FALSE),
  mFilesOnly(PR_FALSE)
{
}

/**
 * Destroy a Songbird directory enumerator object.
 */

sbDirectoryEnumerator::~sbDirectoryEnumerator()
{
  // Finalize the Songbird directory enumerator.
  Finalize();
}


/**
 * Initialize the Songbird directory enumerator.
 */

nsresult
sbDirectoryEnumerator::Initialize()
{
  // Do nothing if already initialized.
  if (mIsInitialized)
    return NS_OK;

  // Create the directory enumerator lock.
  mEnumeratorLock =
    nsAutoLock::NewLock("sbDirectoryEnumerator.mEnumeratorLock");
  NS_ENSURE_TRUE(mEnumeratorLock, NS_ERROR_OUT_OF_MEMORY);

  // Indicate that the directory enumerator has been initialized.
  mIsInitialized = PR_TRUE;

  return NS_OK;
}


/**
 * Finalize the Songbird directory enumerator.
 */

void
sbDirectoryEnumerator::Finalize()
{
  // Indicate that the directory enumerator is no longer initialized.
  mIsInitialized = PR_FALSE;

  // Dispose of the directory enumerator lock.
  if (mEnumeratorLock)
    nsAutoLock::DestroyLock(mEnumeratorLock);
  mEnumeratorLock = nsnull;

  // Clear the directory entries enumeration stack.
  mEntriesEnumStack.Clear();

  // Release object references.
  mNextFile = nsnull;
}


//------------------------------------------------------------------------------
//
// Songbird directory enumerator private services.
//
//------------------------------------------------------------------------------

/**
 * ScanForNextFile
 *
 *   This function scans for the next file in the enumeration and updates the
 * enumerator appropriately.
 */

nsresult
sbDirectoryEnumerator::ScanForNextFile()
{
  PRBool   success;
  nsresult rv;

  // Scan for the next file.
  while (!mNextFile && (mEntriesEnumStack.Count() > 0)) {
    // Get the enum at the top of the stack.
    nsCOMPtr<nsISimpleEnumerator>
      entriesEnum = mEntriesEnumStack.ObjectAt(mEntriesEnumStack.Count() - 1);

    // If there are no more entries, pop the stack and continue.
    PRBool hasMoreElements;
    rv = entriesEnum->HasMoreElements(&hasMoreElements);
    NS_ENSURE_SUCCESS(rv, rv);
    if (!hasMoreElements) {
      mEntriesEnumStack.RemoveObjectAt(mEntriesEnumStack.Count() - 1);
      continue;
    }

    // Get the next file.
    nsCOMPtr<nsISupports> fileEntry;
    rv = entriesEnum->GetNext(getter_AddRefs(fileEntry));
    NS_ENSURE_SUCCESS(rv, rv);
    nsCOMPtr<nsIFile> file = do_QueryInterface(fileEntry, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    // Skip file if it doesn't match enumerator criteria.
    PRBool skipFile = PR_FALSE;
    if (mDirectoriesOnly) {
      PRBool isDirectory;
      rv = file->IsDirectory(&isDirectory);
      NS_ENSURE_SUCCESS(rv, rv);
      if (!isDirectory)
        skipFile = PR_TRUE;
    }
    if (mFilesOnly) {
      PRBool isFile;
      rv = file->IsFile(&isFile);
      NS_ENSURE_SUCCESS(rv, rv);
      if (!isFile)
        skipFile = PR_TRUE;
    }
    if (!skipFile)
      mNextFile = file;

    // If the current file is a directory, and the maximum depth has not been
    // reached, enumerate the directory entries.
    PRBool isDirectory;
    rv = file->IsDirectory(&isDirectory);
    NS_ENSURE_SUCCESS(rv, rv);
    PRUint32 depth = mEntriesEnumStack.Count();
    if (isDirectory && !((mMaxDepth > 0) && (depth >= mMaxDepth))) {
      rv = file->GetDirectoryEntries(getter_AddRefs(entriesEnum));
      NS_ENSURE_SUCCESS(rv, rv);
      success = mEntriesEnumStack.AppendObject(entriesEnum);
      NS_ENSURE_TRUE(success, NS_ERROR_FAILURE);
    }
  }

  return NS_OK;
}


