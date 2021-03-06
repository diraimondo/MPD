/*
 * Copyright 2003-2017 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef MPD_SIMPLE_DATABASE_PLUGIN_HXX
#define MPD_SIMPLE_DATABASE_PLUGIN_HXX

#include "check.h"
#include "db/Interface.hxx"
#include "fs/AllocatedPath.hxx"
#include "db/LightSong.hxx"
#include "Compiler.h"

#include <cassert>

struct ConfigBlock;
struct Directory;
struct DatabasePlugin;
class EventLoop;
class DatabaseListener;
class PrefixedLightSong;

class SimpleDatabase : public Database {
	AllocatedPath path;
	std::string path_utf8;

#ifdef ENABLE_ZLIB
	bool compress;
#endif

	/**
	 * The path where cache files for Mount() are located.
	 */
	AllocatedPath cache_path;

	Directory *root;

	std::chrono::system_clock::time_point mtime;

	/**
	 * A buffer for GetSong() when prefixing the #LightSong
	 * instance from a mounted #Database.
	 */
	mutable PrefixedLightSong *prefixed_light_song;

	/**
	 * A buffer for GetSong().
	 */
	mutable LightSong light_song;

#ifndef NDEBUG
	mutable unsigned borrowed_song_count;
#endif

	SimpleDatabase(const ConfigBlock &block);

	SimpleDatabase(AllocatedPath &&_path, bool _compress);

public:
	static Database *Create(EventLoop &loop, DatabaseListener &listener,
				const ConfigBlock &block);

	gcc_pure
	Directory &GetRoot() {
		assert(root != NULL);

		return *root;
	}

	void Save();

	/**
	 * Returns true if there is a valid database file on the disk.
	 */
	bool FileExists() const {
		return mtime >= std::chrono::system_clock::time_point(std::chrono::system_clock::duration::zero());
	}

	/**
	 * @param db the #Database to be mounted; must be "open"; on
	 * success, this object gains ownership of the given #Database
	 */
	gcc_nonnull_all
	void Mount(const char *uri, Database *db);

	/**
	 * Throws #std::runtime_error on error.
	 */
	gcc_nonnull_all
	void Mount(const char *local_uri, const char *storage_uri);

	gcc_nonnull_all
	bool Unmount(const char *uri);

	/* virtual methods from class Database */
	void Open() override;
	void Close() override;

	const LightSong *GetSong(const char *uri_utf8) const override;
	void ReturnSong(const LightSong *song) const override;

	void Visit(const DatabaseSelection &selection,
		   VisitDirectory visit_directory,
		   VisitSong visit_song,
		   VisitPlaylist visit_playlist) const override;

	void VisitUniqueTags(const DatabaseSelection &selection,
			     TagType tag_type, TagMask group_mask,
			     VisitTag visit_tag) const override;

	DatabaseStats GetStats(const DatabaseSelection &selection) const override;

	std::chrono::system_clock::time_point GetUpdateStamp() const override {
		return mtime;
	}

private:
	void Configure(const ConfigBlock &block);

	void Check() const;

	/**
	 * Throws #std::runtime_error on error.
	 */
	void Load();

	Database *LockUmountSteal(const char *uri);
};

extern const DatabasePlugin simple_db_plugin;

#endif
