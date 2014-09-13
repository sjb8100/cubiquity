#ifndef __CUBIQUITY_VOXELDATABASE_H__
#define __CUBIQUITY_VOXELDATABASE_H__

#include "PolyVoxCore/Impl/TypeDef.h"

#include "PolyVoxCore/Pager.h"
#include "PolyVoxCore/Region.h"

#include "SQLite/sqlite3.h"

#define MINIZ_HEADER_FILE_ONLY
#include "miniz/miniz.c"

#include "Exceptions.h"
#include "WritePermissions.h"

#include <vector>

namespace Cubiquity
{
	/**
	 * Provides an interface for performing paging of data.
	 */
	template <typename VoxelType>
	class VoxelDatabase : public PolyVox::Pager<VoxelType>
	{
	public:
		/// Destructor
		virtual ~VoxelDatabase();

		static VoxelDatabase* createEmpty(const std::string& pathToNewVoxelDatabase);
		static VoxelDatabase* createFromVDB(const std::string& pathToExistingVoxelDatabase, WritePermission writePermission);

		virtual void pageIn(const PolyVox::Region& region, PolyVox::UncompressedBlock<VoxelType>* pBlockData);
		virtual void pageOut(const PolyVox::Region& region, PolyVox::UncompressedBlock<VoxelType>* pBlockData);

		void acceptOverrideBlocks(void);
		void discardOverrideBlocks(void);

		int32_t getPropertyAsInt(const std::string& name, int32_t defaultValue);
		float getPropertyAsFloat(const std::string& name, float defaultValue);
		std::string getPropertyAsString(const std::string& name, const std::string& defaultValue);

		void setProperty(const std::string& name, int value);
		void setProperty(const std::string& name, float value);
		void setProperty(const std::string& name, const std::string& value);

	private:

		/// Constructor
		VoxelDatabase();

		void initialize(void);

		bool getProperty(const std::string& name, std::string& value);

		sqlite3* mDatabase;

		sqlite3_stmt* mSelectBlockStatement;
		sqlite3_stmt* mSelectOverrideBlockStatement;
		
		sqlite3_stmt* mInsertOrReplaceBlockStatement;
		sqlite3_stmt* mInsertOrReplaceOverrideBlockStatement;

		sqlite3_stmt* mSelectPropertyStatement;
		sqlite3_stmt* mInsertOrReplacePropertyStatement;

		// Used as a temporary store into which we compress
		// block data, before passing it to the database.
		std::vector<uint8_t> mCompressedBuffer;
	};

	// Utility function to perform bit rotation.
	template <typename T> 
	T rotateLeft(T val);

	// Allows us to use a Region as a key in the SQLite database.
	uint64_t regionToKey(const PolyVox::Region& region);
}

#include "VoxelDatabase.inl"

#endif //__CUBIQUITY_VOXELDATABASE_H__