//
//  TabulaRasaData.swift
//  Tabula Rasa
//
//  Created by Darren Ford on 13/10/18.
//  Copyright © 2018 Darren Ford. All rights reserved.
//

import Cocoa

class TabulaRasaData: NSObject {
	private let url: URL
	private var cancelled: Bool = false
	private var loading: Bool = false

	private let group = DispatchGroup()
	private let serialQueue = DispatchQueue(label: "com.csvlib.swiftsync")

	enum FileType: NSInteger {
		case csv = 0
		case tsv = 1
	}

	let type: FileType
	private(set) var rawData: [[String]] = [[String]]()

	init(url: URL, type: FileType) {
		self.url = url
		self.type = type
		super.init()
	}

	private func addRecords(record: [String]) {
		self.rawData.append(record)
	}

	private func setCancelled(_ cancelled: Bool) {
		self.serialQueue.sync {
			self.cancelled = cancelled
		}
	}

	private func isActive() -> Bool {
		var isActive = false
		serialQueue.sync {
			isActive = !self.cancelled
		}
		return isActive
	}

	func cancel() {
		if self.loading {
			self.setCancelled(true)
			self.group.wait()
		}
	}

	func loadAsync(fraction: @escaping (CGFloat) -> Void, completion: @escaping () -> Void) -> Bool {
		assert(self.loading == false)

		// Prepare
		self.setCancelled(false)
		self.rawData.removeAll()

		let sep: UnicodeScalar = self.type == .csv ? "," : "\t"
		let separator = Int8(sep.value)

		let source = DSFCSVDataSource(fileURL: self.url, icuCodepage: nil, separator: separator)
		guard let dataSource = source else {
			return false
		}

		DispatchQueue.global(qos: .userInitiated).async { [weak self] in
			if let blockSelf = self {
				blockSelf.load(source: dataSource, completion: completion, fraction: fraction)
			}
		}

		return true
	}

	private func load(source: DSFCSVDataSource, completion: @escaping () -> Void, fraction: @escaping (CGFloat) -> Void) {
		self.loading = true

		self.group.enter()

		DSFCSVParser.parse(with: source,
						   fieldCallback: nil) { (_: UInt, record: [String], complete: CGFloat) -> Bool in
			self.addRecords(record: record)
			DispatchQueue.main.async {
				fraction(complete)
			}
			return self.isActive()
		}

		self.loading = false
		self.group.leave()

		if self.isActive() {
			/// Only call the completion block if we weren't cancelled
			DispatchQueue.main.async {
				completion()
			}
		}
	}
}
